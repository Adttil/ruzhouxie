#ifndef RUZHOUXIE_ASTRICT_HPP
#define RUZHOUXIE_ASTRICT_HPP

#include "constant.hpp"
#include "general.hpp"
//#include "simplify.hpp"
#include "relayout.hpp"
#include "view_interface.hpp"

#include "macro_define.hpp"

namespace rzx 
{
    enum class stricture_t
    {
        none,
        readonly
    };
}

namespace rzx::detail 
{
    template<class T>
    consteval bool is_totally_const()
    {
        if constexpr(not std::is_const_v<std::remove_reference_t<T>>)
        {
            return false;
        }
        else if constexpr(terminal<T>)
        {
            return true;
        }
        else return []<size_t...I>(std::index_sequence<I...>)
        {
            return (... && is_totally_const<child_type<T, I>>());
        }(std::make_index_sequence<child_count<T>>{});
    }

    template<size_t I, class StrictureTable>
    constexpr auto child_stricture_table(const StrictureTable& stricture_table)
    {
        if constexpr(terminal<StrictureTable>)
        {
            return stricture_table;
        }
        else
        {
            return stricture_table | child<I>;
        }
    }

    template<auto StrictureTable, class Shape>
    constexpr auto simplify_stricture_table(Shape shape = {})
    {
        if constexpr(terminal<decltype(StrictureTable)>)
        {
            static_assert(std::same_as<decltype(StrictureTable), stricture_t>);
            return StrictureTable;
        }
        else return []<size_t...I>(std::index_sequence<I...>)
        {
            constexpr auto children = rzx::make_tuple(simplify_stricture_table<StrictureTable | child<I>>(Shape{} | child<I>)...);
            constexpr size_t pad_size = child_count<Shape> > sizeof...(I) ? child_count<Shape> - sizeof...(I) : 0uz;
            constexpr auto padded_children = concat_to_tuple(children, array<stricture_t, pad_size>{});
            if constexpr((... && rzx::equal(padded_children | child<I>, stricture_t::none)))
            {
                return stricture_t::none;
            }
            else if constexpr((... && rzx::equal(padded_children | child<I>, stricture_t::readonly)))
            {
                return stricture_t::readonly;
            }
            else
            {
                return padded_children;
            }
        }(std::make_index_sequence<child_count<decltype(StrictureTable)>>{});
    }
}

namespace rzx 
{
    namespace detail
    {
        template<auto StrictureTable>
        struct astrict_t;
    }

    template<auto StrictureTable>
    inline constexpr detail::astrict_t<StrictureTable> astrict{};

    namespace detail
    {
        template<typename V, auto Stricture>
        struct astrict_view_storage
        {
            RUZHOUXIE(no_unique_address) V                     base;
            RUZHOUXIE(no_unique_address) constant_t<Stricture> stricture;
        };
    }

    template<typename V, auto StrictureTable>
    struct astrict_view : detail::astrict_view_storage<V, StrictureTable>, view_interface<astrict_view<V, StrictureTable>>
    {
        // template<typename Self>
        // constexpr decltype(auto) self(this Self&& self)
        // {
        //     if constexpr(std::is_object_v<Self> && std::is_reference_v<V>)
        //     {
        //         return astrict_view{ FWD(self) };
        //     }
        //     else
        //     {
        //         return FWD(self);
        //     }
        // }

        template<size_t I, typename Self>
        constexpr decltype(auto) get(this Self&& self)
        {            
            constexpr auto child_stricture_table = detail::child_stricture_table<I>(StrictureTable);
            if constexpr (I >= child_count<V>)
            {
                return end();
            }
            else if constexpr(std::is_reference_v<decltype(FWD(self).template base_child<I>())>)
            {
                return unwrap_t<decltype(FWD(self).template base_child<I>() | refer | astrict<child_stricture_table>)>
                {
                    FWD(self).template base_child<I>()
                };
            }
            else if constexpr(detail::is_totally_const<const decltype(FWD(self, base) | child<I>)&>()
                            || equal(child_stricture_table, stricture_t::none))
            {
                return FWD(self).template base_child<I>();
            }
            else
            {
                return unwrap_t<decltype(FWD(self).template base_child<I>() | refer | astrict<child_stricture_table>)>
                {
                    FWD(self).template base_child<I>()
                };
            }
        }

        // template<auto UsageTable, typename Self>
        // constexpr auto simplifier(this Self&& self)
        // {
        //     struct simplifier_t
        //     {
        //         decltype(FWD(self, base)) base;
                
        //         static constexpr auto layout(){ return simplified_layout<decltype(FWD(self, base)), UsageTable>; }

        //         constexpr decltype(auto) data()const
        //         {
        //             return astrict_view<decltype(FWD(base) | rzx::simplified_data<UsageTable>), StrictureTable>
        //             {
        //                 FWD(base) | rzx::simplified_data<UsageTable>
        //             };
        //         }
        //     };
        //     return simplifier_t{ FWD(self, base) };
        // }        

    private:
        template<size_t I, class Self>
        constexpr decltype(auto) base_child(this Self&& self)
        {
            if constexpr(requires{ requires StrictureTable == stricture_t::readonly; })
            {
                return std::as_const(self.base) | child<I>;
            }
            else
            {
                return FWD(self, base) | child<I>;
            }
        }
        // template<auto UsageTable, typename Self>
        // constexpr decltype(auto) simplified_data(this Self&& self)
        // {
        //     return astrict_view<decltype(FWD(self, base) | rzx::simplified_data<UsageTable>), Stricture>
        //     {
        //         FWD(self, base) | rzx::simplified_data<UsageTable>
        //     };
        // }

        // template<auto Usage, derived_from<astrict_view> Self>
        // friend constexpr decltype(auto) get_simplified_layout(type_tag<Self>)
        // {
        //     return rzx::simplified_layout<V>;
        // }
    };

    template<auto StrictureTable>
    struct detail::astrict_t : adaptor_closure<astrict_t<StrictureTable>>
    {
        template<typename V>
        constexpr decltype(auto) operator()(V&& view)const
        {
            constexpr auto simplified_stricture_table = detail::simplify_stricture_table<StrictureTable>(tree_shape<V>);
            //return astrict_view<unwrap_t<V>, Stricture>{ unwrap(FWD(view)) };
            if constexpr(branched<decltype(simplified_stricture_table)>)
            {
                return astrict_view<unwrap_t<V>, simplified_stricture_table>{ unwrap(FWD(view)) };
            }
            else if constexpr(simplified_stricture_table == stricture_t::none)
            {
                return rzx::view<unwrap_t<V>>{ unwrap(FWD(view)) };
            }
            else if constexpr(simplified_stricture_table == stricture_t::readonly)
            {
                if constexpr(detail::is_totally_const<decltype(std::as_const(unwrap(view)))>() && not (std::is_object_v<V> && not terminal<V>))
                {
                    if constexpr(std::is_object_v<unwrap_t<V>>)
                    {
                        return rzx::view<const unwrap_t<V>>{ unwrap(view) };
                    }
                    else
                    {
                        return wrap(std::as_const(unwrap(view)));
                    }
                }
                else
                {
                    return astrict_view<unwrap_t<V>, stricture_t::readonly>{ unwrap(FWD(view)) };
                }
            }
        }
    };
}

namespace rzx::detail 
{
    template<class S>
    constexpr auto make_default_stricture_table(S shape = {})
    {
        if constexpr(terminal<S>)
        {
            return stricture_t{};
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            auto stricture_table = rzx::make_tuple(detail::make_default_stricture_table(shape | child<I>)...);
            static_assert(std::same_as<tree_shape_t<decltype(stricture_table)>, S>);
            return stricture_table;
        }(std::make_index_sequence<child_count<S>>{});
    };

    template<auto Layout, class StrictureTable>
    constexpr auto set_stricture_table_by_layout(StrictureTable& stricture_table)
    {
        if constexpr(indexical<decltype(Layout)>)
        {
            stricture_table | child<Layout> = stricture_t::readonly;
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            (..., set_stricture_table_by_layout<Layout | child<I>>(stricture_table));
        }(std::make_index_sequence<child_count<decltype(Layout)>>{});
    }

    template<auto Layout, size_t I, class S, class StrictureTables>
    constexpr auto set_stricture_table_for_seq(StrictureTables& stricture_tables, S shape = {})
    {
        if constexpr(I == child_count<decltype(Layout)> - 1uz)
        {
            return;
        }
        else
        {
            stricture_tables | child<I> = stricture_tables | child<I + 1uz>;
            auto& stricture_table =  stricture_tables | child<I>;
            set_stricture_table_by_layout<Layout | child<I + 1uz>>(stricture_table);
        }
    }

    // Layout should be normalize by S. 
    template<auto Layout, class S>
    constexpr auto stricture_tables_for_seq(S shape = {})
    {
        constexpr auto count = child_count<decltype(Layout)>;
        constexpr auto init_stricture_tables = array<decltype(make_default_stricture_table(shape)), count>{};
        auto stricture_tables = init_stricture_tables;
        [&]<size_t...I>(std::index_sequence<I...>)
        {
            constexpr auto count = child_count<decltype(Layout)>;
            (..., set_stricture_table_for_seq<Layout, count - I - 1uz>(stricture_tables, shape));
        }(std::make_index_sequence<count>{});
        return stricture_tables;
    }

    // Layout should be normalize by S. 
    template<auto Layout, class S>
    constexpr auto stricture_table_for_sequence(S shape = {})
    {
        return []<size_t...I>(std::index_sequence<I...>)
        {
            constexpr auto child_stricture_tables = stricture_tables_for_seq<Layout, S>();
            return rzx::make_tuple(apply_layout<Layout | child<I>>(child_stricture_tables | child<I>)...);
        }(std::make_index_sequence<child_count<decltype(Layout)>>{});
    }

    template<class StrictureTable>
    constexpr auto combine_stricture_table(const StrictureTable& l, const StrictureTable& r)
    {
        if constexpr(terminal<StrictureTable>)
        {
            return l < r ? r : l;
        }
        else return[&]<size_t...I>(std::index_sequence<I...>)
        {
            return rzx::make_tuple(detail::combine_stricture_table(l | child<I>, r | child<I>)...);
        }(std::make_index_sequence<child_count<StrictureTable>>{});
    }

    // Layout should be normalize by S. 
    template<auto Layout, class S>
    constexpr auto stricture_table_for_children(S shape = {})
    {
        return []<size_t...I>(std::index_sequence<I...>)
        {
            constexpr auto tables = stricture_tables_for_seq<Layout, S>();

            constexpr auto inv_layout = inverse.relayout(Layout);
            constexpr auto inv_tables = stricture_tables_for_seq<inv_layout, S>();

            constexpr size_t last_index = sizeof...(I) - 1uz;

            return rzx::make_tuple(
                apply_layout<Layout | child<I>>(
                    combine_stricture_table(tables | child<I>, inv_tables | child<last_index - I>)
                )...
            );
        }(std::make_index_sequence<child_count<decltype(Layout)>>{});
    }

    template<auto Layout, bool Sequential, class S>
    constexpr auto stricture_table_for_simple_relayout_seperate(S shape = {})
    {
        if constexpr(Sequential)
        {
            return stricture_table_for_sequence<Layout, S>();
        }
        else
        {
            return stricture_table_for_children<Layout, S>();
        }
    }

    template<auto Layout, bool Sequential, bool Borrow, class T>
    constexpr decltype(auto) simple_relayout_seperate(T&& t) noexcept
    {
        static_assert(not terminal<decltype(Layout)>);
        using data_shape_t = tree_shape_t<T>;
        constexpr auto layout = Layout;
        constexpr auto nlayout = detail::normalize_layout2<layout, data_shape_t>();
        constexpr auto stricture_table = detail::stricture_table_for_sequence<nlayout, data_shape_t>();

        if constexpr(Borrow)
        {
            using simplified_t = decltype(FWD(t) | refer | relayout<nlayout>);
            constexpr auto sstricture_table = detail::simplify_stricture_table<stricture_table>(tree_shape<simplified_t>);

            return decltype(FWD(t) | refer | relayout<nlayout> | astrict<sstricture_table>)
            {
                FWD(t) | refer | relayout<nlayout>
                //unwrap_t<decltype(FWD(t) | relayout<nlayout>)>{ FWD(t) }
            };
        }
        else
        {
            using simplified_t = decltype(FWD(t) | relayout<nlayout>);
            constexpr auto sstricture_table = detail::simplify_stricture_table<stricture_table>(tree_shape<simplified_t>);

            return decltype(FWD(t) | relayout<nlayout> | astrict<sstricture_table>)
            {
                FWD(t) | relayout<nlayout>
                //unwrap_t<decltype(FWD(t) | relayout<nlayout>)>{ FWD(t) }
            };
        }
    }

    
}

namespace rzx::detail 
{
    template<auto Layout, class S>
    constexpr auto normalize_layout_for_relayout_seperate(S shape = {})
    {
        if constexpr(indexical<decltype(Layout)>)
        {
            if constexpr(not equal(Layout, indexes_of_whole))
            {
                return Layout;
            }
            else return []<size_t...I>(std::index_sequence<I...>)
            {
                return rzx::make_tuple(array{ I }...);
            }(std::make_index_sequence<child_count<S>>{});
        }
        else return []<size_t...I>(std::index_sequence<I...>)
        {
            return rzx::make_tuple(normalize_layout_for_relayout_seperate<Layout | child<I>, S>()...);
        }(std::make_index_sequence<child_count<decltype(Layout)>>{});
    }

    template<auto Layout, size_t IStep>
    constexpr auto init_after_layout_for_relayout_seperate_step()
    {
        if constexpr(indexical<decltype(Layout)>)
        {
            return array{ Layout[0], IStep, 0uz - 1uz };
        }
        else return []<size_t...I>(std::index_sequence<I...>)
        {
            return rzx::make_tuple(init_after_layout_for_relayout_seperate_step<Layout | child<I>, IStep>()...);
        }(std::make_index_sequence<child_count<decltype(Layout)>>{});
    }

    template<auto Layout>
    constexpr auto init_after_layout_for_relayout_seperate()
    {
        return []<size_t...I>(std::index_sequence<I...>)
        {
            return rzx::make_tuple(init_after_layout_for_relayout_seperate_step<Layout | child<I>, I>()...);
        }(std::make_index_sequence<child_count<decltype(Layout)>>{});
    }

    template<size_t N, size_t I, class T>
    constexpr auto make_tuple_with_single_at(const T& t)
    {
        return [&]<size_t...J, size_t...K>(std::index_sequence<J...>, std::index_sequence<K...>)
        {
            auto foo = tuple<tuple<>>{{}};
            return rzx::make_tuple(foo | child<J - J>..., t, foo | child<K - K>...);
        }(std::make_index_sequence<I>{}, std::make_index_sequence<N - I - 1uz>{});
    }

    template<class Tpl, class...Rest>
    constexpr auto concat_children(const Tpl& tpl, const Rest&...rest)
    {
        return [&]<size_t...I>(std::index_sequence<I...>)
        {
            auto get = [&]<size_t J>()
            {
                return concat_to_tuple(tpl | child<J>, rest | child<J>...);
            };

            return rzx::make_tuple(get.template operator()<I>()...);
        }(std::make_index_sequence<child_count<Tpl>>{});
    }

    template<auto Layout, size_t NChild, typename AfterLayout>
    constexpr auto step_get_children_layout_impl(const array<size_t, NChild>& group_count, array<size_t, NChild>& pos_count, AfterLayout& after_layout)
    {
        if constexpr(indexical<decltype(Layout)>)
        {
            after_layout[1] = group_count[Layout[0]];
            after_layout[2] = pos_count[Layout[0]]++;
            return make_tuple_with_single_at<NChild, Layout[0]>(tuple{ rzx::array_drop<1uz>(Layout) });
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            auto children = rzx::tuple<decltype(detail::step_get_children_layout_impl<Layout | child<I>>(group_count, pos_count, after_layout | child<I>))...>
            {
                detail::step_get_children_layout_impl<Layout | child<I>>(group_count, pos_count, after_layout | child<I>)...
            };
            return detail::concat_children(children | child<I>...);
        }(std::make_index_sequence<child_count<AfterLayout>>{});
    }

    template<auto Layout, size_t NChild, typename AfterLayout>
    constexpr auto step_get_children_layout(array<size_t, NChild>& group_count, AfterLayout& after_layout)
    {
        std::array<size_t, NChild> pos_count{};
        auto result = step_get_children_layout_impl<Layout>(group_count, pos_count, after_layout);
        for(size_t i = 0; i < NChild; ++i)
        {
            if(pos_count[i]) ++group_count[i];
        }

        return [&]<size_t...I>(std::index_sequence<I...>)
        {
            auto optional = []<class...T>(const tuple<T...>& tpl)
            {
                if constexpr(sizeof...(T) == 0uz)
                {
                    return tpl;
                }
                else
                {
                    return tuple<tuple<T...>>{ tpl };
                }
            };
            return rzx::make_tuple(optional(result | child<I>)...);
        }(std::make_index_sequence<NChild>{});
    }

    template<auto Layout, size_t NChild, class AfterLayout>
    constexpr auto get_children_layout(AfterLayout& after_layout)
    {
        return [&]<size_t...I>(std::index_sequence<I...>)
        {
            std::array<size_t, NChild> group_count{};
            auto steps = rzx::tuple<decltype(step_get_children_layout<Layout | child<I>>(group_count, after_layout | child<I>))...>
            {
                step_get_children_layout<Layout | child<I>>(group_count, after_layout | child<I>)...
            };
            return concat_children(steps | child<I>...);
        }(std::make_index_sequence<child_count<decltype(Layout)>>{});
    }

    template<auto Layout, class S>
    constexpr auto normal_relayout_seperate_info(S shape = {})
    {
        constexpr auto nlayout = normalize_layout_for_relayout_seperate<Layout, S>();
        auto after_layout = init_after_layout_for_relayout_seperate<nlayout>();

        auto children_layouts = get_children_layout<Layout, child_count<S>>(after_layout);

        struct info
        {
            decltype(children_layouts) children_layouts;
            decltype(after_layout) after_layout;
        };

        return info{ children_layouts, after_layout };
    }

    template<auto Layout, bool Sequential, bool Borrow, class T>
    constexpr decltype(auto) normal_relayout_seperate(T&& t)
    {
        constexpr auto info = normal_relayout_seperate_info<Layout>(tree_shape<T>);
        constexpr auto children_layouts = info.children_layouts;
        constexpr auto after_layout = info.after_layout;

        if constexpr(Borrow)
        {
            auto get_children_seperate = [&]<size_t...I>(std::index_sequence<I...>)
            {
                return rzx::tuple<decltype(FWD(t) | child<I> | refer | relayout_seperate<children_layouts | child<I>, Sequential>)...>
                {
                    FWD(t) | child<I> | refer | relayout_seperate<children_layouts | child<I>, Sequential> ...
                };
            };
    
            return decltype(get_children_seperate(std::make_index_sequence<child_count<T>>{}) | relayout<after_layout>)
            {
                 get_children_seperate(std::make_index_sequence<child_count<T>>{}) 
            };
        }
        else
        {
            auto get_children_seperate = [&]<size_t...I>(std::index_sequence<I...>)
            {
                return rzx::tuple<decltype(FWD(t) | child<I> | relayout_seperate<children_layouts | child<I>, Sequential>)...>
                {
                    FWD(t) | child<I> | relayout_seperate<children_layouts | child<I>, Sequential> ...
                };
            };

            return decltype(get_children_seperate(std::make_index_sequence<child_count<T>>{}) | relayout<after_layout>)
            {
                 get_children_seperate(std::make_index_sequence<child_count<T>>{}) 
            };
        }
    }
}

namespace rzx 
{
    template<auto Layout, bool Sequential>
    struct detail::relayout_seperate_t_ns::relayout_seperate_t : adaptor_closure<relayout_seperate_t<Layout, Sequential>>
    {
        template<class T>
        constexpr decltype(auto) operator()(T&& t) const
        {
            constexpr auto simplified_layout = detail::seperate_simplify_layout<Layout>(tree_shape<T>);
            return impl<simplified_layout, std::is_reference_v<unwrap_t<T>>>(unwrap(FWD(t)));
        }

        template<auto SimplifiedLayout, bool Borrow, class T>
        static constexpr decltype(auto) impl(T&& t)
        {
            //clang bug(clang 18.1): https://gcc.godbolt.org/z/8KfEo94Kv
            //constexpr auto simplified_layout = detail::seperate_simplify_layout<Layout>(tree_shape<T>);
            if constexpr(terminal<decltype(SimplifiedLayout)>)
            {
                static_assert(not terminal<decltype(SimplifiedLayout)>);
            }
            if constexpr(requires{ FWD(t).template relayout_seperate<SimplifiedLayout, Sequential, Borrow>(custom_t{}); })
            {
                FWD(t).template relayout_seperate<SimplifiedLayout, Sequential, Borrow>(custom_t{});
            }
            else if constexpr(requires{ relayout_seperate<SimplifiedLayout, Sequential, Borrow>(FWD(t), custom_t{}); })
            {
                return relayout_seperate<SimplifiedLayout, Sequential, Borrow>(FWD(t), custom_t{});
            }
            else if constexpr(requires{ FWD(t).template relayout_seperate<SimplifiedLayout, Sequential, Borrow>(); })
            {
                return FWD(t).template relayout_seperate<SimplifiedLayout, Sequential, Borrow>();
            }
            else if constexpr(requires{ relayout_seperate<SimplifiedLayout, Sequential, Borrow>(FWD(t)); })
            {
                return relayout_seperate<SimplifiedLayout, Sequential, Borrow>(FWD(t));
            }
            else if constexpr(simple<T>)
            {
                return detail::simple_relayout_seperate<SimplifiedLayout, Sequential, Borrow>(FWD(t));
            }
            else
            {
                return detail::normal_relayout_seperate<SimplifiedLayout, Sequential, Borrow>(FWD(t));
            }
        }
    };
}

namespace rzx 
{
    inline constexpr auto sequence = relayout_seperate<indexes_of_whole, true>;

    namespace detail
    {
        struct inverse_sequence_t : adaptor_closure<inverse_sequence_t>
        {
            template<branched T>
            constexpr decltype(auto) operator()(T&& t)const
            {
                constexpr auto inverse_layout = []<size_t...I>(std::index_sequence<I...>)
                {
                    return rzx::make_tuple(array{ child_count<T> - 1uz - I }...);
                }(std::make_index_sequence<child_count<T>>{});

                return FWD(t) | relayout_seperate<inverse_layout, true>;
            }
        };
    }

    inline constexpr detail::inverse_sequence_t inverse_sequence{};

    inline constexpr auto seperate = relayout_seperate<indexes_of_whole, false>;
}

#include "macro_undef.hpp"
#endif