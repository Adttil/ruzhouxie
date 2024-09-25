#ifndef RUZHOUXIE_ASTRICT_HPP
#define RUZHOUXIE_ASTRICT_HPP

#include "constant.hpp"
#include "general.hpp"
#include "simplify.hpp"
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
                return FWD(self).template base_child<I>() | astrict<child_stricture_table>;
            }
            else if constexpr(detail::is_totally_const<const decltype(FWD(self, base) | child<I>)&>()
                            || equal(child_stricture_table, stricture_t::none))
            {
                return FWD(self).template base_child<I>();
            }
            else
            {
                return astrict_view<decltype(FWD(self).template base_child<I>()), child_stricture_table>
                {
                    FWD(self).template base_child<I>()
                };
            }
        }

        template<auto UsageTable, typename Self>
        constexpr auto simplifier(this Self&& self)
        {
            struct simplifier_t
            {
                decltype(FWD(self, base)) base;
                
                static constexpr auto layout(){ return simplified_layout<decltype(FWD(self, base)), UsageTable>; }

                constexpr decltype(auto) data()const
                {
                    return astrict_view<decltype(FWD(base) | rzx::simplified_data<UsageTable>), StrictureTable>
                    {
                        FWD(base) | rzx::simplified_data<UsageTable>
                    };
                }
            };
            return simplifier_t{ FWD(self, base) };
        }        

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
                return FWD(view);
            }
            else if constexpr(simplified_stricture_table == stricture_t::readonly)
            {
                if constexpr(detail::is_totally_const<decltype(std::as_const(view))>())
                {
                    return std::as_const(view);
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
    // S should be branched.
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
    // S should be branched.
    template<auto Layout, class S>
    constexpr auto stricture_table_for_use_child_sequentially(S shape = {})
    {
        return []<size_t...I>(std::index_sequence<I...>)
        {
            constexpr auto child_stricture_tables = stricture_tables_for_seq<Layout, S>();
            return rzx::make_tuple(apply_layout<Layout | child<I>>(child_stricture_tables | child<I>)...);
        }(std::make_index_sequence<child_count<decltype(Layout)>>{});
    }
}

namespace rzx 
{
    namespace detail
    {
        struct sequence_t : adaptor_closure<sequence_t>
        {
            template<branched T>
            constexpr decltype(auto) operator()(T&& t)const
            {
                using data_shape_t = tree_shape_t<decltype(FWD(t) | rzx::simplified_data<>)>;
                constexpr auto layout = simplified_layout<T>;
                constexpr auto nlayout = detail::normalize_layout2<layout, data_shape_t>();
                constexpr auto stricture_table = detail::stricture_table_for_use_child_sequentially<nlayout, data_shape_t>();

                using simplified_t = decltype(FWD(t) | simplify<>);

                return astrict_view<simplified_t, stricture_table>{ FWD(t) | simplify<> };
            }

            template<terminal T>
            constexpr tuple<> operator()(T&& t)const
            {
                return {};
            }
        };
    }

    inline constexpr detail::sequence_t sequence{};
}

#include "macro_undef.hpp"
#endif