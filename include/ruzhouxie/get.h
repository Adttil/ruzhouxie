#ifndef RUZHOUXIE_GET_H
#define RUZHOUXIE_GET_H

#include "general.h"
#include "tree_adaptor.h"
#include "array.h"
#include "tuple.h"
#include "math.h"

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

    inline constexpr array<size_t, 0uz> indices_of_whole_view{};

    struct end_t
    {
        end_t() = delete;
    };

    end_t end();

    namespace detail
    {
        template<auto...I>
        struct child_t
//#ifdef _MSC_VER
        { void operator()(); }//make MSVC happy.
//#endif
        ;
    }

    inline namespace functors
    {
        template<auto...I>
        inline constexpr tree_adaptor_closure<detail::child_t<I...>> child{};
    }

    template<typename T>
    struct getter_trait;

    template<typename T>
    using getter = getter_trait<T>::type;

    template<typename T>
    inline constexpr size_t child_count = []<size_t N = 0uz>(this auto&& self)
    {
        if constexpr (std::same_as<decltype(getter<purified<T>>{}.template get<N>(std::declval<T>())), end_t>)
        {
            return N;
        }
        else
        {
            return self.template operator()<N + 1uz>();
        }
    }();
    
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
        template<typename T>
        RUZHOUXIE_INLINE constexpr auto operator()(T&& t)const AS_EXPRESSION
        (
            getter<purified<T>>{}.template get<normalize_index(I, child_count<T>)>(FWD(t))
        )
        
        template<typename T> requires (I.size() == 0uz)
        RUZHOUXIE_INLINE constexpr T&& operator()(T&& t)const noexcept
        {
            return FWD(t);
        }

    private:
        template<typename T, size_t...J>
        RUZHOUXIE_INLINE static constexpr auto impl(T&& t, std::index_sequence<J...>) AS_EXPRESSION
        (
            FWD(t) | child<I[J]...>
        )

    public:    
        template<typename T> requires (I.size() > 0uz)
        RUZHOUXIE_INLINE constexpr auto operator()(T&& t)const AS_EXPRESSION
        (
            impl(FWD(t), std::make_index_sequence<I.size()>{})
        )
    };

    template<auto I, std::integral auto...Rest> 
        requires (std::integral<decltype(I)> || indicesoid<decltype(I)>) && (sizeof...(Rest) > 0)
    struct detail::child_t<I, Rest...>
    {
        template<typename T> 
        RUZHOUXIE_INLINE constexpr auto operator()(T&& t)const AS_EXPRESSION
        (
            FWD(t) | child<I> | child<Rest...>
        )
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
                return (0uz + ... + leaf_count<child_type<T, I>>);
            }(std::make_index_sequence<child_count<T>>{});
        }
    }();

    struct leaf_tag_t{};

    template<typename T>
    inline constexpr auto tree_shape = []
    {
        if constexpr (terminal<T>)
        {
            return leaf_tag_t{};
        }
        else return[]<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(tree_shape<child_type<T, I>>...);
        }(std::make_index_sequence<child_count<T>>{});
    }();

    template<typename T>
    using tree_shape_t = purified<decltype(tree_shape<T>)>;

    template<typename T>
    inline constexpr size_t tensor_rank = []
    {
        if constexpr (terminal<T>)
        {
            return 0uz;
        }
        else return[]<size_t...I>(std::index_sequence<I...>)
        {
            return 1uz + min(tensor_rank<child_type<T, I>>...);
        }(std::make_index_sequence<child_count<T>>{});
    }();

    template<typename T>
    inline constexpr auto tensor_shape = []
    {
        if constexpr (terminal<T>)
        {
            return array<size_t, 0uz>{};
        }
        else return[]<size_t...I>(std::index_sequence<I...>)
        {
            constexpr size_t rank = tensor_rank<T>;
            array<size_t, rank> result{ child_count<T> };

            constexpr auto child_shapes = tuple{ tensor_shape<child_type<T, I>>... };
            for (size_t i = 0uz; i < rank - 1uz; ++i)
            {
                result[i + 1uz] = min((child_shapes | child<I>)[i]...);
            }

            return result;
        }(std::make_index_sequence<child_count<T>>{});
    }();
}

namespace ruzhouxie
{
    struct invalid_getter 
    {
        template<size_t I, typename T>
        end_t get(T&& t)const;
    };

    namespace detail::tag_invoke_getter_ns
    {
        //for adl find.
        template<size_t I>
        void tag_invoke();

        struct tag_invoke_getter
        {
            template<size_t I, typename T>
            RUZHOUXIE_INLINE constexpr auto get(T&& t)const AS_EXPRESSION
            (
                tag_invoke<I>(child<I>, FWD(t))
            )
        };
    }
    using detail::tag_invoke_getter_ns::tag_invoke_getter;

    struct array_getter
    {
        template<size_t I, typename T>
        RUZHOUXIE_INLINE constexpr auto get(T&& t)const noexcept
        {
            if constexpr(I >= std::extent_v<purified<T>>)
            {
                return end();
            }
            else
            {
                return FWD(t)[I];
            }
        }
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
        static RUZHOUXIE_CONSTEVAL choice_t<strategy_t> choose()
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
            else
            {
                return end();
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
            else if constexpr (requires{ type{ {universal_type{}}, universal_type{args}... }; })
            {
                return self.template operator()<true>(universal_type{ args }..., universal_type{});
            }
            else if constexpr (had_success)
            {
                return sizeof...(args);
            }
            else
            {
                return self.template operator()<false>(universal_type{ args }..., universal_type{});
            }
        }();

        template<size_t I, aggregated T>
        constexpr decltype(auto) get(T&& t)const noexcept
        {
            constexpr size_t n = aggregate_member_count<T>;
            if constexpr(I >= n)
            {
                return end();
            }
            else
            {
#include "generate/aggregate_getter_invoker.code"
            }
        };
    };

    template<typename T>
    struct getter_trait
    {
        static RUZHOUXIE_CONSTEVAL auto choose_default_getter() noexcept
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

namespace ruzhouxie::detail
{
    RUZHOUXIE_CONSTEVAL auto concat_to_tuple()
	{
	    return tuple{};
	}

    template<typename T>
    RUZHOUXIE_CONSTEVAL auto concat_to_tuple(T&& t)
	{
	    return [&]<size_t...I>(std::index_sequence<I...>)
		{
			return make_tuple(FWD(t) | child<I>...);
		}(std::make_index_sequence<child_count<T>>{});
	}

    template<typename T1, typename T2>
    RUZHOUXIE_CONSTEVAL auto concat_to_tuple(T1&& t1, T2&& t2)
	{
	    return[&]<size_t...I, size_t...J>(std::index_sequence<I...>, std::index_sequence<J...>)
		{
		    return make_tuple(FWD(t1) | child<I>..., FWD(t2) | child<J>...);
		}(std::make_index_sequence<child_count<T1>>{}, std::make_index_sequence<child_count<T2>>{});
	}

    template<typename T1, typename T2, typename...Rest>
    RUZHOUXIE_CONSTEVAL auto concat_to_tuple(T1&& t1, T2&& t2, Rest&&...rest)
	{
	    return concat_to_tuple(concat_to_tuple(FWD(t1), FWD(t2)), FWD(rest)...);
	}

    template<auto Indices, typename V> requires std::integral<purified<decltype(Indices)>> || indicesoid<decltype(Indices)>
    RUZHOUXIE_CONSTEVAL auto normalize_indices()
    {
        if constexpr(std::integral<purified<decltype(Indices)>>)
        {
            return array{ normalize_index(Indices, child_count<V>) };
        }
        else return []<size_t...I>(std::index_sequence<I...>)
        {
            return array<size_t, Indices.size()>
            {
                normalize_index(Indices[I], child_count<child_type<V, detail::array_take<I>(Indices)>>)...
            };
        }(std::make_index_sequence<Indices.size()>{});
    }

    template<auto Layout, typename V>
    RUZHOUXIE_CONSTEVAL auto normalize_layout()
    {
        if constexpr(indicesoid<decltype(Layout)> || std::integral<purified<decltype(Layout)>>)
        {
            return normalize_indices<Layout, V>();
        }
        else return []<size_t...I>(std::index_sequence<I...>)
        {
            constexpr auto child_relayout = make_tuple(normalize_layout<Layout | child<I>, V>()...);
            constexpr size_t n = child_count<decltype(child_relayout.template get<0uz>())>;

            if constexpr(n > 0uz
                && (... && indicesoid<decltype(child_relayout.template get<I>())>)
                && (... && (n == child_count<decltype(child_relayout.template get<I>())>))
            )
            {
                constexpr auto prefix = array_take<n - 1uz>(child_relayout.template get<0uz>());
                if constexpr((... && (prefix == array_take<n - 1uz>(child_relayout.template get<I>())))
                    && (... && ((child_relayout.template get<I>())[n - 1uz] == I))
                )
                {
                    return prefix;
                }
                else
                {
                    return child_relayout;
                }
            }
            else
            {
                return child_relayout;
            }
        }(std::make_index_sequence<child_count<decltype(Layout)>>{});
    }

    template<auto Seq, typename V>
    RUZHOUXIE_CONSTEVAL auto normalize_sequence()
    {
        return []<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(normalize_layout<Seq | child<I>, V>()...);
        }(std::make_index_sequence<child_count<decltype(Seq)>>{});
    }
}

#include "macro_undef.h"
#endif