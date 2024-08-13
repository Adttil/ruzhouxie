#ifndef RUZHOUXIE_TREE_BASIC_HPP
#define RUZHOUXIE_TREE_BASIC_HPP

#include "child.hpp"
#include "general.hpp"

#include "macro_define.hpp"


namespace rzx 
{
    template<typename V>
    struct view_interface;
    
    namespace detail::make_t_ns
    {
        template<typename T>
        struct make_t;
    }

    template<typename T>
    inline constexpr detail::make_t_ns::make_t<T> make{};

    namespace detail
    {
        template<auto StrictureTable>
        struct restrict_t;
    }

    template<auto StrictureTable>
    inline constexpr detail::restrict_t<StrictureTable> restrict{};

    enum class usage_t
    {
        discard,
        once,
        repeatedly
    };

    namespace detail::simplify_t_ns
    {
        template<auto UsageTable, auto Layout>
        struct simplify_t;
    }

    template<auto UsageTable = usage_t::repeatedly, auto Layout = indexes_of_whole>
    inline constexpr detail::simplify_t_ns::simplify_t<UsageTable, Layout> simplify{};
}

//make
namespace rzx 
{
    template<typename T>
    struct tuple_maker
    {
        template<typename Arg>
        constexpr T operator()(Arg&& arg)
        {
            return [&]<size_t...I>(std::index_sequence<I...>)
            {
                return T{ FWD(arg) | child<I> | make<std::tuple_element_t<I, T>>... };
            }(std::make_index_sequence<std::tuple_size_v<T>>{});
        }
    };

    namespace detail::make_t_ns
    {        
        template<typename T>
        constexpr auto get_maker(type_tag<T>)noexcept
        {
            return tuple_maker<T>{};
        }
    }

    template<typename T>
    struct detail::make_t_ns::make_t : adaptor_closure<make_t<T>>
    {
        template<typename Arg>
        constexpr T operator()(Arg&& arg)const
        {
            if constexpr(terminal<T>)
            {
                return static_cast<T>(FWD(arg));
            }
            else if constexpr(std::same_as<std::remove_cv_t<Arg>, T> && requires{ T{ FWD(arg) }; })
            {
                return T{ FWD(arg) };
            }
            else if constexpr(requires{ get_maker(type_tag<T>{}); })
            {
                return get_maker(type_tag<T>{})(FWD(arg));
            }
            else
            {
                static_assert(false, "maker for T not found.");
            }
        }
    };
}

//simplify
namespace rzx 
{
    namespace detail::simplify_t_ns
    {        
        template<auto UsageTable, auto Layout>
        void simplify();
    }

    template<auto UsageTable, auto Layout>
    struct detail::simplify_t_ns::simplify_t : adaptor_closure<simplify_t<UsageTable, Layout>>
    {
        template<typename T>
        constexpr decltype(auto) operator()(T&& t)const
        {
            if constexpr(requires{ FWD(t).template simplify<UsageTable, Layout>(custom_t{}); })
            {
                return FWD(t).template simplify<UsageTable, Layout>(custom_t{});
            }
            else if constexpr(requires{ simplify<UsageTable, Layout>(FWD(t), custom_t{}); })
            {
                return simplify<UsageTable, Layout>(FWD(t), custom_t{});
            }
            else if constexpr(requires{ FWD(t).template simplify<UsageTable, Layout>(); })
            {
                return FWD(t).template simplify<UsageTable, Layout>();
            }
            else if constexpr(requires{ simplify<UsageTable, Layout>(FWD(t)); })
            {
                return simplify<UsageTable, Layout>(FWD(t));
            }
            else
            {
                return FWD(t);
            }
        }
    };
}

#include "macro_undef.hpp"
#endif