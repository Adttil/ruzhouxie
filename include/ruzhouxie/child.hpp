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
            else if constexpr(sizeof...(args) == 0 && requires{ type{ {universal_type{}} }; })
            {
                return self.template operator()<true>(universal_type{});
            }
            else if constexpr (sizeof...(args) != 0 && requires{ type{ universal_type{}, universal_type{args}... }; })
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
            //consteval coused error in msvc.
            template<typename T>
            static constexpr choice_t<strategy_t> choose()
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

        //void self();
    }

    inline namespace functors
    {
        //msvc bug: https://developercommunity.visualstudio.com/t/MSVC-cannot-correctly-recognize-NTTP-in/10722592
        //template<indexical auto...I>
        //inline constexpr detail::child_t<rzx::to_indexes(I...)> child{};

        template<indexical auto...I>
        inline constexpr auto child = detail::child_t<rzx::to_indexes(I...)>{};
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
                // if constexpr(requires{ FWD(t).self(custom_t{}); })
                // {
                //     return FWD(t).self(custom_t{});
                // }
                // else if constexpr(requires{ self(FWD(t), custom_t{}); })
                // {
                //     return self(FWD(t), custom_t{});
                // }
                // else if constexpr(requires{ FWD(t).self(); })
                // {
                //     return FWD(t).self();
                // }
                // else if constexpr(requires{ self(FWD(t)); })
                // {
                //     return self(FWD(t));
                // }
                // else
                // {
                //     return FWD(t);
                // }
            }
            else if constexpr(Indexes.size() == 1uz)
            {
                static_assert(child_count<T> > 0);
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
    using tree_shape_t = std::remove_const_t<decltype(tree_shape<T>)>;

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

    namespace detail
    {
        template<class T>
        constexpr bool is_aggregate_tree()
        {
            return []<size_t...I>(std::index_sequence<I...>) -> bool
            {
                return (... && (detail::get_ns::get_t<I>::template choose<T>().strategy == detail::get_ns::strategy_t::aggregate));
            }(std::make_index_sequence<child_count<T>>{});
        }
    }

    template<class T>
    concept aggregate_tree = std::is_aggregate_v<std::remove_cvref_t<T>> && detail::is_aggregate_tree<T>();

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
        }(std::make_index_sequence<child_count<S>>{});
    }

    // template<class F, class T>
    // constexpr decltype(auto) apply(F&& fn, T&& t)
    // {
    //     return [&]<size_t...I>(std::index_sequence<I...>) -> decltype(auto)
    //     {
    //         return FWD(fn)(FWD(t) | child<I>...);
    //     }(std::make_index_sequence<child_count<T>>{});
    // }
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

    template<indexical auto Indices, class Shape>
    constexpr auto normalize_indices(Shape shape)
    {
        if constexpr(std::integral<decltype(Indices)>)
        {
            return array{ normalize_index(Indices, child_count<Shape>) };
        }
        else return []<size_t...I>(std::index_sequence<I...>)
        {
            return array<size_t, Indices.size()>
            {
                normalize_index(Indices[I], child_count<child_type<Shape, rzx::array_take<I>(Indices)>>)...
            };
        }(std::make_index_sequence<Indices.size()>{});
    }

    template<auto Layout, class Shape>
    constexpr auto fold_layout(Shape shape = {})
    {
        if constexpr(indexical_array<decltype(Layout)> || std::integral<decltype(Layout)>)
        {
            return normalize_indices<Layout>(shape);
        }
        else return []<size_t...I>(std::index_sequence<I...>)
        {
            constexpr auto child_relayout = rzx::make_tuple(fold_layout<Layout | child<I>>(Shape{})...);
            constexpr size_t n = child_count<decltype(child_relayout | child<0uz>)>;

            if constexpr(n > 0uz
                && (... && indexical_array<decltype(child_relayout | child<I>)>)
                && (... && (n == child_count<decltype(child_relayout | child<I>)>))
            )
            {
                constexpr auto prefix = rzx::array_take<n - 1uz>(child_relayout | child<0uz>);
                if constexpr((... && (prefix == rzx::array_take<n - 1uz>(child_relayout | child<I>)))
                    && (... && ((child_relayout | child<I>)[n - 1uz] == I))
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

    template<auto Layout, class Shape>
    constexpr auto unfold_layout(Shape shape = {})
    {
        if constexpr(indexical<decltype(Layout)>)
        {
            constexpr auto indexes = normalize_indices<Layout>(shape);
            using child_shape_t = child_type<Shape, indexes>;
            if constexpr(terminal<child_shape_t>)
            {
                return indexes;
            }
            else return [&]<size_t...I>(std::index_sequence<I...>)
            {
                constexpr auto indexes = normalize_indices<Layout>(shape);
                return rzx::make_tuple(unfold_layout<array_cat(indexes, array{ I })>(shape)...);
            }(std::make_index_sequence<child_count<child_shape_t>>{});
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return rzx::make_tuple(unfold_layout<Layout | child<I>>(shape)...);
        }(std::make_index_sequence<child_count<decltype(Layout)>>{});
    }

    //unfold_layout<layout, shape>() == unfold_layout_by_relayouted_shape(layout, apply_layout<layout>(shape{}))
    template<class Shape, class Layout>
    constexpr auto unfold_layout_by_relayouted_shape(const Layout& layout, Shape shape = {})
    {
        if constexpr(terminal<Shape>)
        {
            static_assert(indexical<Layout>, "Invalid layout.");
            return to_indexes(layout);
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            if constexpr(not indexical<Layout>)
            {
                static_assert(child_count<Shape> == child_count<Layout>, "Invalid layout.");
                return rzx::make_tuple(unfold_layout_by_relayouted_shape(layout | child<I>, shape | child<I>)...);
            }
            else
            {
                auto indexes = to_indexes(layout);
                return rzx::make_tuple(unfold_layout_by_relayouted_shape(array_cat(indexes, array{ I }) , shape | child<I>)...);
            }
        }(std::make_index_sequence<child_count<Shape>>{});
    }

    template<typename TLayout, size_t N>
    constexpr auto layout_add_prefix(const TLayout& layout, const array<size_t, N>& prefix)
    {
        if constexpr(indexical_array<TLayout>)
        {
            return rzx::array_cat(prefix, layout);
        }
        else return[&]<size_t...I>(std::index_sequence<I...>)
        {
            return rzx::make_tuple(detail::layout_add_prefix(layout | child<I>, prefix)...);
        }(std::make_index_sequence<child_count<TLayout>>{});
    }
}

namespace rzx::detail
{
    template<auto Layout>
    constexpr auto apply_layout(const auto& view)
    {
        using layout_type = decltype(Layout);
        if constexpr(indexical<layout_type>)
        {
            return view | child<Layout>;
        }
        else return[&]<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(apply_layout<Layout | child<I>>(view)...);
        }(std::make_index_sequence<child_count<layout_type>>{});
    }

    /*struct pending_indexes{};
    
    template<auto Layout>
    constexpr auto apply_incomplete_layout(auto view, auto pad)
    {
        using layout_type = decltype(Layout);
        if constexpr(std::same_as<layout_type, pending_indexes>)
        {
            return pad;
        }
        else if constexpr(indexical<layout_type>)
        {
            return view | child<Layout>;
        }
        else return[&]<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(apply_incomplete_layout<Layout | child<I>>(view)...);
        }(std::make_index_sequence<child_count<layout_type>>{});
    }

    struct failed_t{};

    template<typename L1, typename L2>
    constexpr auto combine_2complex_layout(L1 layout1, L2 layout2)
    {
        static_assert(child_count<L2> != 0uz && child_count<L1> != 0uz, "invalid layout!");
        
        if constexpr(child_count<L1> > child_count<L2>)
        {
            return combine_2complex_layout(layout2, layout1);
        }
        else
        {    
            auto pad = array<pending_indexes, child_count<L2> - child_count<L1>>{};
            auto padding_layout1 = concat_to_tuple(layout1, pad);

            auto child0 = combine_layout(layout1 | child<0uz>, layout2 | child<0uz>);
            if constexpr(std::same_as<decltype(child0), failed_t>)
            {
                return failed_t{};
            }
            else if constexpr(child_count<L2> == 1uz)
            {
                return make_tuple(child0);
            }
            else
            {
                auto rest = combine_layout(drop_to_tuple<1uz>(padding_layout1), drop_to_tuple<1uz>(layout2));
                if constexpr(std::same_as<decltype(rest), failed_t>)
                {
                    return failed_t{};
                }
                else
                {
                    return concat_to_tuple(make_tuple(child0), rest);
                }
            }
        }
    }

    template<typename L1, typename L2>
    constexpr auto combine_2layout(L1 layout1, L2 layout2)
    {
        if constexpr(std::same_as<L1, pending_indexes>)
        {
            return layout2;
        }
        else if constexpr(indexical<L1>)
        {
            if constexpr(std::same_as<L2, pending_indexes>)
            {
                return layout1;
            }
            else
            {
                return failed_t{};
            }
        }
        else if constexpr(indexical<L2>)
        {
            return failed_t{};
        }
        else if constexpr(std::same_as<pending_indexes, L2>)
        {
            return layout1;
        }
        else
        {
            return combine_2complex_layout(layout1, layout2);
        }
    }

    constexpr auto combine_layout()
    {
        return pending_indexes{};
    }

    template<typename S1, typename S2>
    constexpr auto min_common_drived_tree_shape(S1 shape1 = {}, S2 shape2 = {})
    {
        if constexpr(child_count<S1> > child_count<S2>)
        {
            return min_common_drived_tree_shape(shape2, shape1);
        }
        else if constexpr(terminal<S1>)
        {
            return shape2;
        }
        else return[&]<size_t...I>(std::index_sequence<I...>)
        {
            auto pad = array<leaf_tag_t, child_count<S2> - child_count<S1>>{};
            auto padding_shape1 = concat_to_tuple(shape1, pad);

            return make_tuple(min_common_drived_tree_shape(padding_shape1 | child<I>, shape2 | child<I>)...);
        }(std::make_index_sequence<child_count<S2>>{});
    }

    template<typename L, typename...Rest>
    constexpr auto combine_layout(L layout, Rest...rest)
    {
        auto rest_result = combine_layout(rest...);
        if constexpr(std::same_as<decltype(rest_result), failed_t>)
        {
            return failed_t{};
        }
        else
        {
            return combine_2layout(layout, rest_result);
        }
    }

    template<auto Layout, auto prefix = indexes_of_whole>
    constexpr auto inverse_layout()
    {
        using layout_type = decltype(Layout);
        if constexpr(indexical_array<layout_type>)
        {
            if constexpr(child_count<layout_type> == 0uz)
            {
                return prefix;
            }
            else
            {
                auto pad = array<pending_indexes, Layout | child<0uz>>{};
                return concat_to_tuple(pad, make_tuple(inverse_layout<array_drop<1uz>(Layout), prefix>()));
            }
        }
        else return[]<size_t...I>(std::index_sequence<I...>)
        {
            return combine_layout(inverse_layout<Layout | child<I>, concat_array(prefix, array{ I })>()...);
        }(std::make_index_sequence<child_count<layout_type>>{});
    }*/
}

namespace rzx::detail
{
    template<class L, class CL, class R, class RL>
    struct storage_type;

    template<class T>
    struct storage_type<T&, const T&, T&&, const T&&>
    {
        using type = T;
    };

    template<class T>
    struct storage_type<T, T, T, T>
    {
        using type = T;
    };

    template<class T>
    struct storage_type<T&, T&, T&, T&>
    {
        using type = T&;
    };

    template<class T>
    struct storage_type<const T&, const T&, const T&, const T&>
    {
        using type = const T&;
    };

    template<class T>
    struct storage_type<T&, T&, T&&, T&&>
    {
        using type = T&&;
    };

    template<class T>
    struct storage_type<const T&, const T&, const T&&, const T&&>
    {
        using type = const T&&;
    };

    template<size_t I, class T>
    struct tuple_element_by_child
    : storage_type<child_type<T&, I>, child_type<const T&, I>, child_type<T&&, I>, child_type<const T&&, I>>
    {};

    template<size_t I, class T>
    using tuple_element_t_by_child = tuple_element_by_child<I, T>::type;

    template<class T>
    consteval bool is_valid_for_tuple_element_by_child()
    {
        return []<size_t...I>(std::index_sequence<I...>)
        {
            return (... && requires{ typename tuple_element_by_child<I, T>::type; });
        }(std::make_index_sequence<child_count<T>>{});
    }
}

#include "macro_undef.hpp"
#endif