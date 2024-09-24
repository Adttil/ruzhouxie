#ifndef RUZHOUXIE_MAKE_HPP
#define RUZHOUXIE_MAKE_HPP

#include "child.hpp"
#include "constant.hpp"
#include "general.hpp"
#include "relayout.hpp"
#include "astrict.hpp"
#include "simplify.hpp"
#include "view_interface.hpp"

#include "macro_define.hpp"

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

    template<auto Layout, class S>
    constexpr auto stricture_tables_for_seq(S shape = {})
    {
        constexpr auto nlayout = detail::normalize_layout2<Layout>(S{});
        constexpr auto count = child_count<decltype(nlayout)>;
        constexpr auto init_stricture_tables = array<decltype(make_default_stricture_table(shape)), count>{};
        auto stricture_tables = init_stricture_tables;
        [&]<size_t...I>(std::index_sequence<I...>)
        {
            constexpr auto nlayout = detail::normalize_layout2<Layout>(S{});
            constexpr auto count = child_count<decltype(nlayout)>;
            (..., set_stricture_table_for_seq<nlayout, count - I - 1uz>(stricture_tables, shape));
        }(std::make_index_sequence<count>{});
        return stricture_tables;
    }
}

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
    struct tuple_maker
    {
        template<typename Arg>
        constexpr T operator()(Arg&& arg)const
        {
            return [&]<size_t...I>(std::index_sequence<I...>)
            {
                auto simplifier = FWD(arg) | get_simplifier<>;
                auto&& data = FWD(simplifier).data();
                using data_shape_t = tree_shape_t<decltype(data)>;
                constexpr auto layout = detail::simplify_layout<simplifier.layout(), data_shape_t>();
                constexpr auto stricture_tables = detail::stricture_tables_for_seq<layout, data_shape_t>();

                return T{ 
                    FWD(data)
                    | astrict<stricture_tables | child<I>> 
                    | refer
                    | relayout<layout> 
                    | make<std::tuple_element_t<I, T>, I>... 
                };
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
                    auto simplifier = FWD(arg) | get_simplifier<>;
                    auto&& data = FWD(simplifier).data();
                    using data_shape_t = tree_shape_t<decltype(data)>;
                    constexpr auto layout = detail::simplify_layout<simplifier.layout(), data_shape_t>();
                    constexpr auto stricture_tables = detail::stricture_tables_for_seq<layout, data_shape_t>();

                    (..., fn( 
                        FWD(data)
                        | astrict<stricture_tables | child<I>> 
                        | refer
                        | relayout<layout> 
                        | child<I> 
                    ));
                }(std::make_index_sequence<child_count<Arg>>{});
            }
        }; 
    }

    inline constexpr detail::for_each_t for_each{};
}

#include "macro_undef.hpp"
#endif