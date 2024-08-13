#ifndef RUZHOUXIE_TREE_BASIC_HPP
#define RUZHOUXIE_TREE_BASIC_HPP

#include "child.hpp"
#include "constant.hpp"
#include "general.hpp"
#include "view_interface.hpp"

#include "macro_define.hpp"


namespace rzx 
{    
    namespace detail::make_t_ns
    {
        template<typename T>
        struct make_t;
    }

    template<typename T>
    inline constexpr detail::make_t_ns::make_t<T> make{};

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

    enum class stricture_t
    {
        none,
        readonly
    };

    namespace detail
    {
        template<auto StrictureTable>
        struct restrict_t;
    }

    template<auto StrictureTable>
    inline constexpr detail::restrict_t<StrictureTable> restrict{};

    namespace detail
    {
        template<auto Layout>
        struct relayout_t;
    }

    template<auto Layout>
    inline constexpr detail::relayout_t<Layout> layout{};
}

namespace rzx::detail
{
    
    
    template<auto Layout>
    constexpr auto apply_layout(auto view)
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

    struct pending_indexes{};
    
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

    //==================================================================

    

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

    template<typename U, typename L, typename R>
    constexpr void inverse_apply_layout_on_usage_at(const U& usage, const L& layout, R& result)
    {
        if constexpr(indexical<L>)
        {
            auto&& result_usage = result | child<layout>;
            if constexpr(terminal<decltype(result_usage)>)
            {
                result_usage = max(result_usage, usage);
            }
            else return [&]<size_t...I>(std::index_sequence<I...>)
            {
                (..., inverse_apply_layout_on_usage_at(usage, indexes_of_whole, result_usage | child<I>));
            }(std::make_index_sequence<child_count<decltype(result_usage)>>{});
        }
        else return[&]<size_t...I>(std::index_sequence<I...>)
        {
            (..., inverse_apply_layout_on_usage_at(usage | child<I>, layout | child<I>, result));
        }(std::make_index_sequence<child_count<L>>{});
    }

    template<typename U, typename L, typename S>
    constexpr auto inverse_apply_layout_on_usage(const U& usage, const L& layout, const S& shape)
    {
        auto result = make_tree_of_same_value(usage_t::discard, shape);
        inverse_apply_layout_on_usage_at(usage, layout, result);
        return result;
    }
    //===================================================================    

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
    }
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

    template<typename V>
    concept simple = std::same_as<decltype(std::declval<V>() | simplify<>), V&&>;
}

//restrict
namespace rzx
{
    namespace detail
    {
        template<typename V, auto Stricture>
        struct restrict_view_storage
        {
            RUZHOUXIE(no_unique_address) V                     base;
            RUZHOUXIE(no_unique_address) constant_t<Stricture> stricture;
        };
    }

    template<typename V, auto Stricture>
    struct restrict_view : detail::restrict_view_storage<V, Stricture>, view_interface<restrict_view<V, Stricture>>
    {
        template<size_t I, typename Self>
        constexpr decltype(auto) get(this Self&& self)
        {            
            if constexpr (I >= child_count<V>)
            {
                return end();
            }
            else if constexpr(terminal<child_type<V, I>>)
            {
                if constexpr(std::is_object_v<decltype(FWD(self, base) | child<I>)>)
                {
                    return FWD(self, base) | child<I>;
                }
                else if constexpr(std::same_as<decltype(Stricture), stricture_t>)
                {
                    return FWD(self, base) | child<I> | restrict<Stricture>;
                }
                else if constexpr(I >= child_count<decltype(Stricture)>)
                {
                    return FWD(self, base) | child<I>;
                }
                else
                {
                    return FWD(self, base) | child<I> | restrict<Stricture | child<I>>;
                } 
            }
            else if constexpr(std::same_as<decltype(Stricture), stricture_t>)
            {                
                //can simplify.
                return restrict_view<decltype(FWD(self, base) | child<I>), Stricture>
                {
                    FWD(self, base) | child<I>
                };
            }
            else if constexpr(I >= child_count<decltype(Stricture)>)
            {
                return FWD(self, base) | child<I>;
            }
            else
            {
                //can simplify.
                return restrict_view<decltype(FWD(self, base) | child<I>), Stricture | child<I>>
                {
                    FWD(self, base) | child<I>
                };
            } 
        }

        template<auto Usage, auto Layout, typename Self>
        constexpr decltype(auto) simplify(this Self&& self)
        {
            return restrict_view<decltype(FWD(self, base) | rzx::simplify<Usage, Layout>), apply_layout<Layout>(Stricture)>
            {
                FWD(self, base) | rzx::simplify<Usage, Layout>
            };
        }
    };


    template<auto Stricture>
    struct detail::restrict_t
    {
        template<typename V>
        constexpr decltype(auto) operator()(V&& view)const
        {
            if constexpr(terminal<V>)
            {
                if constexpr(Stricture == stricture_t::none)
                {
                    return FWD(view);
                }
                else if constexpr(Stricture == stricture_t::readonly)
                {
                    return std::as_const(view);
                }
            }
            else
            {
                return restrict_view<V&&, Stricture>{ FWD(view) };
            }
        }
    };
}

//relayout
namespace rzx
{
    namespace detail 
    {
        template<typename V, auto Layout>
        struct relayout_view_storage
        {
            RUZHOUXIE(no_unique_address) V base;
            RUZHOUXIE(no_unique_address) constant_t<Layout> layout;
        };

        enum class relayout_view_child_strategy
        {
            none,
            indices,
            child,
            relayout
        };
    }

    template<typename V, auto Layout>
    struct relayout_view : detail::relayout_view_storage<V, Layout>
    {
    private:
        template<size_t I, typename Self>
        static consteval choice_t<detail::relayout_view_child_strategy> child_choose()
        {
            using strategy_t = detail::relayout_view_child_strategy;
            using layout_type = decltype(Layout);

            if constexpr(indexical_array<layout_type>)
            {
                if constexpr(I < child_count<child_type<V, Layout>>)
                {
                    return { strategy_t::indices, noexcept(FWD(std::declval<Self>(), base) | child<Layout> | child<I>) };
                }
                else
                {
                    return { strategy_t::none, true };
                }
            }
            if constexpr(I >= child_count<layout_type>)
            {
                return { strategy_t::none, true };
            }
            else if constexpr(indexical_array<child_type<layout_type, I>>)
            {
                if constexpr(requires{ FWD(std::declval<Self>(), base) | child<Layout | child<I>>; })
                {
                    return { strategy_t::child, noexcept(FWD(std::declval<Self>(), base) | child<Layout | child<I>>) };
                }
                else
                {
                    return { strategy_t::none, true };
                }
            }
            else 
            {
                return { strategy_t::relayout, true };
            }
        }
    
    public:
        template<size_t I, typename Self>
        constexpr decltype(auto) get(this Self&& self)
            noexcept(child_choose<I, Self>().nothrow)
        {
            using strategy_t = detail::relayout_view_child_strategy;
            constexpr strategy_t strategy = child_choose<I, Self>().strategy;
            
            if constexpr (strategy == strategy_t::none)
            {
                return end();
            }
            else if constexpr(strategy == strategy_t::indices)
            {
                return FWD(self, base) | child<Layout> | child<I>;
            }
            else if constexpr(strategy == strategy_t::child)
            {
                constexpr auto index_pack = Layout | child<I>;
                return FWD(self, base) | child<index_pack>;
            }
            else if constexpr(strategy == strategy_t::relayout)
            {
                return relayout_view<decltype(FWD(self, base)), Layout | child<I>>
                {
                    FWD(self, base)
                };
            }
            else
            {
                static_assert(strategy == strategy_t::none, "Should not reach.");
            }
        }

        template<auto Usage, auto Layout_, typename Self>
        constexpr decltype(auto) simplify(this Self&& self)
        {
            constexpr auto layout = detail::apply_layout<Layout_>(Layout);
            if constexpr(simple<V>)
            {
                return relayout_view<decltype(FWD(self, base)), layout>{ FWD(self, base) };
            }
            else
            {
                constexpr auto base_usage = detail::inverse_apply_layout_on_usage(Usage, Layout, tree_shape<V>);

                return FWD(self, base) | rzx::simplify<base_usage, layout>;
            }
        }
    };

    template<typename V, auto Layout>
    relayout_view(V, constant_t<Layout>) -> relayout_view<V, Layout>;
}

#include "macro_undef.hpp"
#endif