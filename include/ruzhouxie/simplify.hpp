#ifndef RUZHOUXIE_SIMPLIFY_HPP
#define RUZHOUXIE_SIMPLIFY_HPP

#include "child.hpp"
#include "general.hpp"

#include "macro_define.hpp"

namespace rzx 
{  
    enum class usage_t
    {
        discard,
        once,
        repeatedly
    };

    constexpr usage_t operator+(usage_t l, usage_t r)
    {
        auto result = std::to_underlying(l) + std::to_underlying(r);
        return result > std::to_underlying(usage_t::repeatedly) ? usage_t::repeatedly : static_cast<usage_t>(result);
    }
}

namespace rzx::detail 
{
    template<auto Layout, typename U, typename R>
    constexpr void inverse_apply_layout_on_usage_at(const U& usage, R& result)
    {
        if constexpr(indexical<decltype(Layout)>)
        {
            auto&& result_usage = result | child<Layout>;
            if constexpr(terminal<decltype(result_usage)>)
            {
                result_usage = result_usage + usage;
            }
            else return [&]<size_t...I>(std::index_sequence<I...>)
            {
                (..., inverse_apply_layout_on_usage_at<indexes_of_whole>(usage, result_usage | child<I>));
            }(std::make_index_sequence<child_count<decltype(result_usage)>>{});
        }
        else return[&]<size_t...I>(std::index_sequence<I...>)
        {
            (..., inverse_apply_layout_on_usage_at<Layout | child<I>>(usage | child<I>, result));
        }(std::make_index_sequence<child_count<decltype(Layout)>>{});
    }

    template<auto Layout, typename U, typename S>
    constexpr auto inverse_apply_layout_on_usage(const U& usage, const S& shape)
    {
        auto result = make_tree_of_same_value(usage_t::discard, shape);
        inverse_apply_layout_on_usage_at<Layout>(usage, result);
        return result;
    }

    template<typename UsageTable, typename Shape>
    constexpr auto normalize_usage(const UsageTable& usage_table, const Shape& shape)
    {
        if constexpr(terminal<Shape>)
        {
            static_assert(std::same_as<UsageTable, usage_t>, "Invalid usage table.");
            return usage_table;
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            if constexpr(branched<UsageTable>)
            {
                constexpr size_t n_pad = child_count<Shape> > child_count<UsageTable> ? child_count<Shape> - child_count<UsageTable> : 0uz;
                constexpr auto pad = array<usage_t, n_pad>{};
                const auto padded_usage_table = concat_to_tuple(usage_table, pad);
                return rzx::make_tuple(normalize_usage(padded_usage_table | child<I>, shape | child<I>)...);
            }
            else
            {
                static_assert(std::same_as<UsageTable, usage_t>, "Invalid usage table.");
                return rzx::make_tuple(normalize_usage(usage_table, shape | child<I>)...);
            }
        }(std::make_index_sequence<child_count<Shape>>{});
    }
}

namespace rzx 
{  
    namespace detail::simplified_data_t_ns
    {
        template<auto UsageTable>
        struct simplified_data_t;
        
        template<auto UsageTable>
        void simplified_data();
    }

    template<auto UsageTable = usage_t::repeatedly>
    inline constexpr detail::simplified_data_t_ns::simplified_data_t<UsageTable> simplified_data{};

    template<auto UsageTable>
    struct detail::simplified_data_t_ns::simplified_data_t : adaptor_closure<simplified_data_t<UsageTable>>
    {
        template<typename T>
        constexpr decltype(auto) operator()(T&& t)const
        {
            constexpr auto normalized_usage_table = detail::normalize_usage(UsageTable, tree_shape<T>);

            if constexpr(requires{ FWD(t).template simplified_data<normalized_usage_table>(custom_t{}); })
            {
                return FWD(t).template simplified_data<normalized_usage_table>(custom_t{});
            }
            else if constexpr(requires{ simplified_data<normalized_usage_table>(FWD(t), custom_t{}); })
            {
                return simplified_data<normalized_usage_table>(FWD(t), custom_t{});
            }
            else if constexpr(requires{ FWD(t).template simplified_data<normalized_usage_table>(); })
            {
                return FWD(t).template simplified_data<normalized_usage_table>();
            }
            else if constexpr(requires{ simplified_data<normalized_usage_table>(FWD(t)); })
            {
                return simplified_data<normalized_usage_table>(FWD(t));
            }
            else if constexpr(terminal<T>)
            {
                return FWD(t);
            }
            else return [&]<size_t...I>(std::index_sequence<I...>)
            {
                constexpr auto normalized_usage_table = detail::normalize_usage(UsageTable, tree_shape<T>);

                return rzx::tuple<decltype(FWD(t) | child<I> | rzx::simplified_data<normalized_usage_table | child<I>>)...>
                {
                    FWD(t) | child<I> | rzx::simplified_data<normalized_usage_table | child<I>>...
                };
            }(std::make_index_sequence<child_count<T>>{});
        }
    };
}

namespace rzx 
{  
    namespace detail::simplified_layout_t_ns
    {
        void get_simplified_layout();
        
        template<class T>
        constexpr auto simplified_layout()
        {
            if constexpr(requires{ get_simplified_layout(type_tag<T>{}); })
            {
                return get_simplified_layout(type_tag<T>{});
            }
            else if constexpr(terminal<T>)
            {
                return indexes_of_whole;
            }
            else return [&]<size_t...I>(std::index_sequence<I...>)
            {
                return rzx::make_tuple(simplified_layout<child_type<T, I>>()...);
            }(std::make_index_sequence<child_count<T>>{});
        };
    }

    template<class T>
    inline constexpr auto simplified_layout = detail::simplified_layout_t_ns::simplified_layout<T>();
}

namespace rzx 
{  

    namespace detail::simplify_t_ns
    {
        template<auto UsageTable, auto Layout>
        struct simplify_t;
        
        template<auto UsageTable, auto Layout>
        void simplify();
    }

    template<auto UsageTable = usage_t::repeatedly, auto Layout = indexes_of_whole>
    inline constexpr detail::simplify_t_ns::simplify_t<UsageTable, Layout> simplify{};

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