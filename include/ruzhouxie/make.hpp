#ifndef RUZHOUXIE_MAKE_HPP
#define RUZHOUXIE_MAKE_HPP

#include "child.hpp"
#include "constant.hpp"
#include "general.hpp"
#include "relayout.hpp"
#include "astrict.hpp"
//#include "simplify.hpp"
#include "view_interface.hpp"

#include "macro_define.hpp"

namespace rzx 
{
    namespace detail::make_t_ns
    {
        template<typename T, indexical_array auto indexes>
        struct make_t;
    }

    template<typename T, indexical auto...indexes>
    inline constexpr auto make = detail::make_t_ns::make_t<T, to_indexes(indexes...)>{};
    
    template<typename T>
    struct sequence_maker
    {
        template<typename Arg>
        constexpr T operator()(Arg&& arg)const
        {
            return [&]<size_t...I>(std::index_sequence<I...>)
            {
                auto&& seq = FWD(arg) | sequence;
                return T{ FWD(seq) | make<std::tuple_element_t<I, T>, I>... };
            }(std::make_index_sequence<std::tuple_size_v<T>>{});
        }
    };

    template<typename T>
    struct inverse_sequence_maker
    {
        template<typename Arg>
        constexpr T operator()(Arg&& arg)const
        {
            return [&]<size_t...I>(std::index_sequence<I...>)
            {
                auto&& seq = FWD(arg) | inverse_sequence;
                constexpr size_t last_index = sizeof...(I) - 1;
                return T{ FWD(seq) | make<std::tuple_element_t<I, T>, last_index - I>... };
            }(std::make_index_sequence<std::tuple_size_v<T>>{});
        }
    };

    template<typename T>
    struct children_maker
    {
        template<typename Arg>
        constexpr T operator()(Arg&& arg)const
        {
            return [&]<size_t...I>(std::index_sequence<I...>)
            {
                auto&& seq = FWD(arg) | seperate;
                return T{ FWD(seq) | make<std::tuple_element_t<I, T>, I>... };
            }(std::make_index_sequence<std::tuple_size_v<T>>{});
        }
    };

    template<typename T>
    struct aggregate_maker
    {
        template<typename Arg>
        constexpr T operator()(Arg&& arg)const
        {
            return [&]<size_t...I>(std::index_sequence<I...>)
            {
                auto&& seq = FWD(arg) | sequence;
                return T{ { FWD(seq) | make<detail::tuple_element_t_by_child<I, T>, I> }... };
            }(std::make_index_sequence<child_count<T>>{});
        }
    };

    namespace detail::make_t_ns
    {        
        template<typename T>
        constexpr auto get_maker(type_tag<T>)noexcept
        {
            if constexpr(aggregate_tree<T>)
            {
                return aggregate_maker<T>{};
            }
            else if constexpr(requires{ std::tuple_size<T>::value; })
            {
                return children_maker<T>{};
            }
            else
            {

            }
        }
    }

    template<typename T, indexical_array auto indexes>
    struct detail::make_t_ns::make_t : adaptor_closure<make_t<T, indexes>>
    {
        template<typename Arg>
        constexpr T operator()(Arg&& arg)const
        {
            if constexpr(terminal<child_type<Arg, indexes>>)
            {
                return FWD(arg) | child<indexes>;
            }
            else if constexpr(std::same_as<std::remove_cvref_t<child_type<Arg, indexes>>, T> && requires{ T{ FWD(arg) | child<indexes> }; })
            {
                return FWD(arg) | child<indexes>;
            }
            else if constexpr(requires{ get_maker(type_tag<T>{}); })
            {
                return get_maker(type_tag<T>{})(FWD(arg) | child<indexes>);
            }
            else
            {
                static_assert(false, "maker for T not found.");
            }
        }
    };
}

namespace rzx 
{
    namespace detail 
    {
        struct for_each_t : adaptor<for_each_t>
        {
            template<typename Arg, typename Fn>
            constexpr void result(Arg&& arg, Fn&& fn)const
            {
                [&]<size_t...I>(std::index_sequence<I...>)
                {
                    auto&& seq = FWD(arg) | sequence;
                    (..., fn(FWD(seq) | child<I>));
                }(std::make_index_sequence<child_count<Arg>>{});
            }
        }; 
    }

    inline constexpr detail::for_each_t for_each{};

    namespace detail 
    {
        struct apply_t : adaptor<apply_t>
        {
            template<typename Args, typename Fn>
            constexpr decltype(auto) result(Args&& args, Fn&& fn)const
            {
                return [&]<size_t...I>(std::index_sequence<I...>) -> decltype(auto)
                {
                    auto&& seperate_args = FWD(args) | seperate;
                    return FWD(fn)(FWD(seperate_args) | child<I>...);
                }(std::make_index_sequence<child_count<Args>>{});
            }
        };
    }

    inline constexpr detail::apply_t apply{};

    namespace detail 
    {
        struct for_each_children_t : adaptor<for_each_children_t>
        {
            template<typename Arg, typename Fn>
            constexpr void result(Arg&& arg, Fn&& fn)const
            {
                for_each(FWD(arg), apply(fn));
            }
        }; 
    }

    inline constexpr detail::for_each_children_t for_each_children{};
}

namespace rzx 
{
    template<class...T>
    constexpr auto get_maker(type_tag<tuple<T...>>)
    {
        return sequence_maker<tuple<T...>>{};
    }

    template<class...T>
    constexpr auto get_maker(type_tag<std::tuple<T...>>)
    {
        return inverse_sequence_maker<std::tuple<T...>>{};
    }

    template<class T, size_t N>
    constexpr auto get_maker(type_tag<array<T, N>>)
    {
        return sequence_maker<array<T, N>>{};
    }
}

#include "macro_undef.hpp"
#endif