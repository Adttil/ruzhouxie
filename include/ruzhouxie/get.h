#ifndef RUZHOUXIE_GET_H
#define RUZHOUXIE_GET_H

#include "general.h"
#include "tree_adaptor.h"
#include "array.h"

#include "macro_define.h"

namespace ruzhouxie
{
    template<typename T>
    concept indicesoid = requires(purified<T> t, size_t i)
    {
        requires std::integral<typename purified<T>::value_type>;
        std::tuple_size_v<purified<T>>;
        { t[i] } -> std::same_as<typename purified<T>::value_type&>;
    };

    namespace detail
    {
        template<auto...I>
        struct child_t;
    }

    inline namespace functors
    {
        template<auto...I>
        inline constexpr tree_adaptor_closure<detail::child_t<I...>> child{};
    }

    template<typename T>
    inline constexpr size_t child_count = []<size_t N = 0uz>(this auto&& self)
    {
        if constexpr (requires{ { std::declval<T>() | child<N> } -> concrete; })
        {
            return self.template operator()<N + 1uz>();
        }
        else
        {
            return N;
        }
    }();

    template<typename T>
    struct getter_trait;

    template<typename T>
    using getter = getter_trait<T>::type;

    template<>
    struct detail::child_t<>
    {
        template<typename T>
        RUZHOUXIE_INLINE constexpr T&& operator()(T&& t)const noexcept
        {
            return FWD(t);
        }
    };

    template<auto I> requires std::integral<decltype(I)> || indicesoid<decltype(I)>
    struct detail::child_t<I>
    {
        static constexpr bool is_index = indicesoid<purified<decltype(I)>>;

        template<typename T>
        RUZHOUXIE_INLINE constexpr auto operator()(T&& t)const
            AS_EXPRESSION(getter<purified<T>>{}.template get<static_cast<size_t>(I)>(FWD(t)))
        
        template<typename T> requires is_index && (I.size() == 0uz)
        RUZHOUXIE_INLINE constexpr T&& operator()(T&& t)const noexcept
        {
            return FWD(t);
        }

    private:
        template<typename T, size_t...J>
        RUZHOUXIE_INLINE static constexpr auto impl(T&& t, std::index_sequence<J...>) 
            AS_EXPRESSION(FWD(t) | child<static_cast<size_t>(I[J]) ...>)

    public:    
        template<typename T> requires is_index && (I.size() > 0uz)
        RUZHOUXIE_INLINE constexpr auto operator()(T&& t)const
            AS_EXPRESSION(impl(FWD(t), std::make_index_sequence<I.size()>{}))
    };

    template<std::integral auto I, std::integral auto...Rest>
    struct detail::child_t<I, Rest...>
    {
        template<typename T> 
        RUZHOUXIE_INLINE constexpr auto operator()(T&& t)const
            AS_EXPRESSION(getter<purified<T>>{}.template get<I>(FWD(t)) | child<Rest...>)
    };

    template<typename T, auto...I>
    using child_type = decltype(std::declval<T>() | child<I...>);

    template<typename T>
    concept terminal = child_count<T> == 0;

    template<typename T>
    concept branched = not terminal<T>;

    template<typename T>
    inline constexpr size_t leaf_count = []
        {
            if constexpr (terminal<T>)
            {
                return 1uz;
            }
            else
            {
                return[]<size_t...I>(std::index_sequence<I...>)
                {
                    return (0uz + ... + leaf_count<decltype(std::declval<T>() | child<I>())>);
                }(std::make_index_sequence<child_count<T>>{});
            }
        }();

    template<typename T>
    inline constexpr auto tree_shape = []
        {
            if constexpr (terminal<T>)
            {
                return array{ '{', '}' };
            }
            else return[]<size_t...I>(std::index_sequence<I...>)
            {
                return detail::concat_array<array{ '{' }, tree_shape<decltype(child<I>(std::declval<T>()))>..., array{ '}' }>();
            }(std::make_index_sequence<child_count<T>>{});
        }();

    template<typename T>
    inline constexpr size_t tensor_rank = []
        {
            if constexpr (terminal<T>)
            {
                return 0uz;
            }
            else return[]<size_t...I>(std::index_sequence<I...>)
            {
                auto child_ranks = array{ tensor_rank<child_type<T, I>>... };
                size_t child_rank_min = std::numeric_limits<size_t>::max();
                for (size_t child_rank : child_ranks)
                {
                    if (child_rank < child_rank_min)
                    {
                        child_rank_min = child_rank;
                    }
                }
                return 1uz + child_rank_min;
            }(std::make_index_sequence<child_count<T>>{});
        }();

    template<typename T>
    inline constexpr auto tensor_shape = []
        {
            if constexpr (terminal<T>)
            {
                return array<size_t, 0>{};
            }
            else return[]<size_t...I>(std::index_sequence<I...>)
            {
                constexpr size_t rank = tensor_rank<T>;
                array<size_t, rank> result{ child_count<T> };

                auto child_shapes = array{ detail::array_take<rank - 1>(tensor_shape<child_type<T, I>>)... };

                for (size_t i = 0uz; i < rank - 1uz; ++i)
                {
                    size_t child_axis_length_min = std::numeric_limits<size_t>::max();
                    for (const auto& child_shape : child_shapes)
                    {
                        if (child_shape[i] < child_axis_length_min)
                        {
                            child_axis_length_min = child_shape[i];
                        }
                    }
                    result[i + 1uz] = child_axis_length_min;
                }

                return result;
            }(std::make_index_sequence<child_count<T>>{});
        }();
}

namespace ruzhouxie
{
    struct invalid_getter {};

    namespace detail::tag_invoke_getter_ns
    {
        //for adl find.
        template<size_t I>
        void tag_invoke();

        struct tag_invoke_getter
        {
            template<size_t I, typename T>
            RUZHOUXIE_INLINE constexpr auto get(T&& t)const AS_EXPRESSION(tag_invoke<I>(child<I>, FWD(t)))
        };
    }
    using detail::tag_invoke_getter_ns::tag_invoke_getter;

    struct array_getter
    {
        template<size_t I, typename T> requires (I < std::extent_v<purified<T>>)
            RUZHOUXIE_INLINE constexpr auto get(T&& t)const AS_EXPRESSION(FWD(t)[I])
    };

    struct member_getter
    {
        template<size_t I, typename T>
        RUZHOUXIE_INLINE constexpr auto get(T&& t)const AS_EXPRESSION(FWD(t).template get<I>())
    };

    struct adl_getter
    {
        template<size_t I, typename T>
        RUZHOUXIE_INLINE constexpr auto get(T&& t)const AS_EXPRESSION(get<I>(FWD(t)))
    };

    struct tuple_like_getter
    {
        enum strategy_t
        {
            none,
            member,
            adl
        };

        template<size_t I, typename T>
        static consteval choice_t<strategy_t> choose()
        {
            using std::get;//for some std::get wich can not find by adl.
            if constexpr (not requires{ requires (I < size_t{ std::tuple_size<purified<T>>::value }); })
            {
                return { strategy_t::none };
            }
            else if constexpr (requires{ std::declval<T>().template get<I>(); })
            {
                return { strategy_t::member, noexcept(std::declval<T>().template get<I>()) };
            }
            else if constexpr (requires{ get<I>(std::declval<T>()); })
            {
                return { strategy_t::adl, noexcept(get<I>(std::declval<T>())) };
            }
            else
            {
                return { strategy_t::none };
            }
        }

        template<size_t I, typename T>
        RUZHOUXIE_INLINE constexpr decltype(auto) get(T&& t)const
            noexcept(choose<I, T&&>().nothrow)
            requires(choose<I, T&&>().strategy != none)
        {
            constexpr strategy_t strategy = choose<I, T&&>().strategy;
            if constexpr (strategy == strategy_t::member)
            {
                return FWD(t).template get<I>();
            }
            else if constexpr (strategy == strategy_t::adl)
            {
                using std::get;
                return get<I>(FWD(t));
            }
        }
    };

    struct aggregate_getter
    {
        struct universal_type
        {
            //Can not use "requires" in clang here.
            //https://github.com/llvm/llvm-project/issues/76415
            template <typename T, typename = std::enable_if_t<std::is_copy_constructible_v<T>>>
            operator T&();

            template <typename T, typename = std::enable_if_t<std::is_move_constructible_v<T>>>
            operator T&&();

            template <typename T, typename = std::enable_if_t<!std::is_copy_constructible_v<T> && !std::is_move_constructible_v<T>>>
            operator T();
        };

        static constexpr size_t aggregate_supported_to_get_max_size = 64uz;

        template <aggregated T>
        static constexpr size_t aggregate_member_count = []<bool had_success = false>(this auto && self, auto...args)
        {
            using type = purified<T>;
            if constexpr (sizeof...(args) > aggregate_supported_to_get_max_size)
            {
                return 0uz;
            }
            else if constexpr (requires{ type{ universal_type{ args }...,  {universal_type{}} }; })
            {
                return self.template operator() < true > (universal_type{ args }..., universal_type{});
            }
            else if constexpr (had_success)
            {
                return sizeof...(args);
            }
            else
            {
                return self.template operator() < false > (universal_type{ args }..., universal_type{});
            }
        }();

        template<size_t I, aggregated T> requires (I < aggregate_member_count<T>)
        constexpr decltype(auto) get(T&& t)const noexcept
        {
            constexpr size_t n = aggregate_member_count<T>;

#include "generate/aggregate_getter_invoker.code"

        };
    };

    template<typename T>
    struct getter_trait
    {
        static consteval auto choose_default_getter() noexcept
        {
            if constexpr (requires{ tag_invoke_getter{}.get<0uz>(std::declval<T>()); })
            {
                return tag_invoke_getter{};
            }
            else if constexpr (std::is_bounded_array_v<purified<T>>)
            {
                return array_getter{};
            }
            else if constexpr (requires{ std::tuple_size<purified<T>>::value; })
            {
                return tuple_like_getter{};
            }
            else if constexpr (aggregated<T>)
            {
                return aggregate_getter{};
            }
            else
            {
                return invalid_getter{};
            }
        }

        using type = decltype(choose_default_getter());
    };
}

#include "macro_undef.h"
#endif