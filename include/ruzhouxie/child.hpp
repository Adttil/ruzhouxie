#ifndef RUZHOUXIE_CHILD_HPP
#define RUZHOUXIE_CHILD_HPP

#include "general.hpp"
#include "adaptor_closure.hpp"
#include "array.hpp"
#include "tuple.hpp"
#include "math.hpp"

#include "macro_define.hpp"

namespace rzx
{
    template<typename T>
    concept indexical_array = requires(std::remove_cvref_t<T> t, size_t i)
    {
        requires std::integral<typename std::remove_cvref_t<T>::value_type>;
        std::tuple_size_v<std::remove_cvref_t<T>>;
        { t[i] } -> std::same_as<typename std::remove_cvref_t<T>::value_type&>;
    };

    template<typename T>
    concept indexical = std::integral<T> || indexical_array<T>;

    inline constexpr array<size_t, 0uz> indexes_of_whole{};
    
    constexpr auto to_indexes(indexical auto...indexes)noexcept
    {
        if constexpr(sizeof...(indexes) == 0uz)
        {
            return indexes_of_whole;
        }
        else if constexpr(sizeof...(indexes) > 1uz)
        {
            return rzx::array_cat(to_indexes(indexes)...);
        }
        else if constexpr(indexical_array<decltype((..., indexes))>)
        {
            return (..., indexes);
        }
        else
        {
            return array{ indexes... };
        }
    }

    struct end_t
    {
        end_t() = delete;
    };

    end_t end();
    
    inline constexpr size_t auto_supported_aggregate_max_size = 64uz;

    namespace detail::get_ns
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

        template <class T>
        inline constexpr size_t aggregate_member_count = []<bool had_success = false>(this auto && self, auto...args)
        {
            using type = std::remove_cvref_t<T>;
            if constexpr (sizeof...(args) > auto_supported_aggregate_max_size)
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

        template<size_t I, class T>
        constexpr decltype(auto) aggregate_get(T&& t) noexcept
        {
            constexpr size_t n = aggregate_member_count<T>;
            if constexpr(I >= n)
            {
                return end();
            }
            else
            {
#include "code_generate/aggregate_getter_invoker.code"
            }
        };

        template<size_t I>
        void get();

        enum strategy_t
        {
            none,
            array,
            tagged_member,
            tagged_adl,
            member,
            adl,
            aggregate
        };

        template<size_t I>
        struct get_t
        {
        private:
            template<typename T>
            consteval static choice_t<strategy_t> choose()
            {
                using type = std::remove_cvref_t<T>;
                if constexpr(std::is_bounded_array_v<type>)
                {
                    if constexpr(I < std::extent_v<type>)
                    {
                        return { strategy_t::array, true };
                    }
                    else
                    {
                        return { strategy_t::none, true };
                    }
                }
                else if constexpr(requires{ std::declval<T>().template get<I>(custom_t{}); })
                {
                    return { strategy_t::tagged_member, noexcept(std::declval<T>().template get<I>(custom_t{})) };
                }
                else if constexpr(requires{ get<I>(std::declval<T>(), custom_t{}); })
                {
                    return { strategy_t::tagged_adl, noexcept(get<I>(std::declval<T>(), custom_t{})) };
                }
                else if constexpr(requires{ requires (I >= std::tuple_size<type>::value); })
                {
                    return { strategy_t::none, true };
                }
                else if constexpr(requires{ std::declval<T>().template get<I>(); })
                {
                    return { strategy_t::member, noexcept(std::declval<T>().template get<I>()) };
                }
                else if constexpr(requires{ get<I>(std::declval<T>()); })
                {
                    return { strategy_t::adl, noexcept(get<I>(std::declval<T>())) };
                }
                else if constexpr(std::is_aggregate_v<type>)
                {
                    return { strategy_t::aggregate, true };
                }
                else
                {
                    return { strategy_t::none, true };
                }
            }

        public:
            template<typename T>
            constexpr decltype(auto) operator()(T&& t)const
            noexcept(choose<T>().nothrow)
            {
                constexpr strategy_t strategy = choose<T>().strategy;

                if constexpr(strategy == strategy_t::none)
                {
                    return end();
                }
                else if constexpr(strategy == strategy_t::array)
                {
                    return FWD(t)[I];
                }
                else if constexpr(strategy == strategy_t::tagged_member)
                {
                    return FWD(t).template get<I>(custom_t{});
                }
                else if constexpr(strategy == strategy_t::tagged_adl)
                {
                    return get<I>(FWD(t), custom_t{});
                }
                else if constexpr(strategy == strategy_t::member)
                {
                    return FWD(t).template get<I>();
                }
                else if constexpr(strategy == strategy_t::adl)
                {
                    return get<I>(FWD(t));
                }
                else if constexpr(strategy == strategy_t::aggregate)
                {
                    return aggregate_get<I>(FWD(t));
                }
            }
        };
    }

    namespace detail 
    {
        template<size_t I>
        inline constexpr get_ns::get_t<I> get{};
    }

    template<typename T>
    inline constexpr size_t child_count = []<size_t N = 0uz>(this auto&& self)
    {
        if constexpr (std::same_as<decltype(detail::get<N>(std::declval<T>())), end_t>)
        {
            return N;
        }
        else
        {
            return self.template operator()<N + 1uz>();
        }
    }();
    
    namespace detail
    {
        template<indexical_array auto Indexes>
        struct child_t;
    }

    inline namespace functors
    {
        template<indexical auto...I>
        inline constexpr detail::child_t<to_indexes(I...)> child{};
    }
    
    template<indexical_array auto Indexes>
    struct detail::child_t : adaptor_closure<child_t<Indexes>>
    {
        template<typename T>
        constexpr decltype(auto) operator()(T&& t)const
        {
            if constexpr(Indexes.size() == 0uz)
            {
                return FWD(t);
            }
            else if constexpr(Indexes.size() == 1uz)
            {
                return get<normalize_index(Indexes[0], child_count<T>)>(FWD(t));
            }
            else
            {
                return get<normalize_index(Indexes[0], child_count<T>)>(FWD(t)) | child<array_drop<1uz>(Indexes)>;
            }
        }
    };

    template<typename T, indexical auto...I>
    using child_type = decltype(std::declval<T>() | child<I...>);

    template<typename T>
    concept terminal = child_count<T> == 0uz;

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
            return rzx::make_tuple(tree_shape<child_type<T, I>>...);
        }(std::make_index_sequence<child_count<T>>{});
    }();

    template<typename T>
    using tree_shape_t = decltype(tree_shape<T>);

    template<typename T>
    inline constexpr size_t tensor_rank = []
    {
        if constexpr (terminal<T>)
        {
            return 0uz;
        }
        else return[]<size_t...I>(std::index_sequence<I...>)
        {
            return 1uz + rzx::min(tensor_rank<child_type<T, I>>...);
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
                result[i + 1uz] = rzx::min((child_shapes | child<I>)[i]...);
            }

            return result;
        }(std::make_index_sequence<child_count<T>>{});
    }();

    template<typename S, typename T>
    constexpr auto make_tree_of_same_value(const T& value, S shape = {})
    {
        if constexpr(terminal<S>)
        {
            return value;
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return rzx::make_tuple(rzx::make_tree_of_same_value(value, shape | child<I>)...);
        }();
    }

    template<class F, class T>
    constexpr decltype(auto) apply(F&& fn, T&& t)
    {
        return [&]<size_t...I>(std::index_sequence<I...>) -> decltype(auto)
        {
            return FWD(fn)(FWD(t) | child<I>...);
        }(std::make_index_sequence<child_count<T>>{});
    }
}

namespace rzx::detail
{
    constexpr auto concat_to_tuple()
	{
	    return tuple{};
	}

    template<typename T>
    constexpr auto concat_to_tuple(T&& t)
	{
	    return [&]<size_t...I>(std::index_sequence<I...>)
		{
			return rzx::make_tuple(FWD(t) | child<I>...);
		}(std::make_index_sequence<child_count<T>>{});
	}

    template<typename T1, typename T2>
    constexpr auto concat_to_tuple(T1&& t1, T2&& t2)
	{
	    return[&]<size_t...I, size_t...J>(std::index_sequence<I...>, std::index_sequence<J...>)
		{
		    return rzx::make_tuple(FWD(t1) | child<I>..., FWD(t2) | child<J>...);
		}(std::make_index_sequence<child_count<T1>>{}, std::make_index_sequence<child_count<T2>>{});
	}

    template<typename T1, typename T2, typename...Rest>
    constexpr auto concat_to_tuple(T1&& t1, T2&& t2, Rest&&...rest)
	{
	    return detail::concat_to_tuple(detail::concat_to_tuple(FWD(t1), FWD(t2)), FWD(rest)...);
	}

    template<size_t N, typename V>
    constexpr auto drop_to_tuple(const V& view)
	{
	    return [&]<size_t...I>(std::index_sequence<I...>)
		{
		    return rzx::make_tuple(view | child<I + N>()...);
		}(std::make_index_sequence<child_count<V> - N>{});
	}

    template<indexical auto Indices, typename V>
    constexpr auto normalize_indices()
    {
        if constexpr(std::integral<decltype(Indices)>)
        {
            return array{ normalize_index(Indices, child_count<V>) };
        }
        else return []<size_t...I>(std::index_sequence<I...>)
        {
            return array<size_t, Indices.size()>
            {
                normalize_index(Indices[I], child_count<child_type<V, rzx::array_take<I>(Indices)>>)...
            };
        }(std::make_index_sequence<Indices.size()>{});
    }

    template<auto Layout, typename V>
    constexpr auto normalize_layout()
    {
        if constexpr(indexical_array<decltype(Layout)> || std::integral<decltype(Layout)>)
        {
            return normalize_indices<Layout, V>();
        }
        else return []<size_t...I>(std::index_sequence<I...>)
        {
            constexpr auto child_relayout = rzx::make_tuple(normalize_layout<Layout | child<I>, V>()...);
            constexpr size_t n = child_count<decltype(child_relayout.template get<0uz>())>;

            if constexpr(n > 0uz
                && (... && indexical_array<decltype(child_relayout.template get<I>())>)
                && (... && (n == child_count<decltype(child_relayout.template get<I>())>))
            )
            {
                constexpr auto prefix = rzx::array_take<n - 1uz>(child_relayout.template get<0uz>());
                if constexpr((... && (prefix == rzx::array_take<n - 1uz>(child_relayout.template get<I>())))
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
    constexpr auto normalize_sequence()
    {
        return []<size_t...I>(std::index_sequence<I...>)
        {
            return rzx::make_tuple(normalize_layout<Seq | child<I>, V>()...);
        }(std::make_index_sequence<child_count<decltype(Seq)>>{});
    }
}

#include "macro_undef.hpp"
#endif