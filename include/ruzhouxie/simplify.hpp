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
    namespace detail::get_simplifier_t_ns
    {
        template<auto UsageTable>
        struct get_simplifier_t;
        
        template<auto UsageTable>
        void simplifier();
        
        template<class T, auto UsageTable>
        constexpr bool is_simple()
        {
            constexpr bool no_custom = not bool
            {
                requires{ std::declval<T>().template simplifier<UsageTable>(custom_t{}); }
                ||
                requires{ simplifier<UsageTable>(std::declval<T>(), custom_t{}); }
                ||
                requires{ std::declval<T>().template simplifier<UsageTable>(); }
                ||
                requires{ simplifier<UsageTable>(std::declval<T>()); }
            };
            return [&]<size_t...I>(std::index_sequence<I...>)
            {
                return (no_custom && ... && is_simple<child_type<T, I>, UsageTable | child<I>>());
            }(std::make_index_sequence<child_count<T>>{});
        }
    }

    template<auto UsageTable = usage_t::repeatedly>
    inline constexpr detail::get_simplifier_t_ns::get_simplifier_t<UsageTable> simplifier{};

    template<class T, auto UsageTable = usage_t::repeatedly>
    concept simple = detail::get_simplifier_t_ns::is_simple<T, detail::normalize_usage(UsageTable, tree_shape<T>)>();

    template<class T, auto UsageTable>
    struct trivial_simplifier
    {
        T&& t;

        static constexpr auto layout()
        {
            return [&]<size_t...I>(std::index_sequence<I...>)
            {
                return rzx::make_tuple(
                    detail::layout_add_prefix(decltype(std::declval<child_type<T, I>>() | simplifier<UsageTable | child<I>>)::layout(), array{ I })...
                );
            }(std::make_index_sequence<child_count<T>>{});
        }

        constexpr auto data()const
        {
            return [&]<size_t...I>(std::index_sequence<I...>)
            {
                return rzx::tuple<decltype((FWD(t) | child<I> | rzx::simplifier<UsageTable | child<I>>).data())...>
                {
                    (FWD(t) | child<I> | rzx::simplifier<UsageTable | child<I>>).data()...
                };
            }(std::make_index_sequence<child_count<T>>{});
        }
    };

    template<auto UsageTable>
    struct detail::get_simplifier_t_ns::get_simplifier_t : adaptor_closure<get_simplifier_t<UsageTable>>
    {
        template<typename T, auto NormalizedUsage = detail::normalize_usage(UsageTable, tree_shape<T>)>
        constexpr auto operator()(T&& t)const
        {
            //clang bug(clang 18.1): https://gcc.godbolt.org/z/8KfEo94Kv
            //constexpr auto normalized_usage_table =detail::normalize_usage(UsageTable, tree_shape<T>);

            if constexpr(requires{ FWD(t).template simplifier<NormalizedUsage>(custom_t{}); })
            {
                return FWD(t).template simplifier<NormalizedUsage>(custom_t{});
            }
            else if constexpr(requires{ simplifier<NormalizedUsage>(FWD(t), custom_t{}); })
            {
                return simplifier<NormalizedUsage>(FWD(t), custom_t{});
            }
            else if constexpr(requires{ FWD(t).template simplifier<NormalizedUsage>(); })
            {
                return FWD(t).template simplifier<NormalizedUsage>();
            }
            else if constexpr(requires{ simplifier<NormalizedUsage>(FWD(t)); })
            {
                return simplifier<NormalizedUsage>(FWD(t));
            }
            else if constexpr(simple<T, NormalizedUsage>)
            {
                struct simplifier_t
                {
                    T&& t;

                    static constexpr auto layout(){ return indexes_of_whole; }

                    constexpr T&& data()const{ return FWD(t); }
                };
                
                return simplifier_t{ FWD(t) };
            }
            else
            { 
                return trivial_simplifier<T, NormalizedUsage>{ FWD(t) };
            }
        }
    };

    namespace detail
    {
        template<auto UsageTable>
        struct simplified_data : adaptor_closure<simplified_data<UsageTable>>
        {
            template<typename T>
            constexpr decltype(auto) operator()(T&& t)const
            {
                return (FWD(t) | simplifier<UsageTable>).data();
            }
        };
    }

    template<auto UsageTable = usage_t::repeatedly>
    inline constexpr detail::simplified_data<UsageTable> simplified_data{};

    template<class T, auto UsageTable = usage_t::repeatedly>
    inline constexpr auto simplified_layout = decltype(std::declval<T>() | simplifier<UsageTable>)::layout();
}

// namespace rzx 
// {  
//     namespace detail::simplified_data_t_ns
//     {
//         template<auto UsageTable>
//         struct simplified_data_t;
        
//         template<auto UsageTable>
//         void simplified_data();
        
//         template<class T, auto UsageTable>
//         constexpr bool is_simple()
//         {
//             constexpr bool no_custom = not bool
//             {
//                 requires{ std::declval<T>().template simplified_data<UsageTable>(custom_t{}); }
//                 ||
//                 requires{ simplified_data<UsageTable>(std::declval<T>(), custom_t{}); }
//                 ||
//                 requires{ std::declval<T>().template simplified_data<UsageTable>(); }
//                 ||
//                 requires{ simplified_data<UsageTable>(std::declval<T>()); }
//             };
//             return [&]<size_t...I>(std::index_sequence<I...>)
//             {
//                 return (no_custom && ... && is_simple<child_type<T, I>, UsageTable | child<I>>());
//             }(std::make_index_sequence<child_count<T>>{});
//         }
//     }

//     template<auto UsageTable = usage_t::repeatedly>
//     inline constexpr detail::simplified_data_t_ns::simplified_data_t<UsageTable> simplified_data{};

//     template<class T, auto UsageTable = usage_t::repeatedly>
//     concept simple = detail::simplified_data_t_ns::is_simple<T, detail::normalize_usage(UsageTable, tree_shape<T>)>();

//     template<auto UsageTable>
//     struct detail::simplified_data_t_ns::simplified_data_t : adaptor_closure<simplified_data_t<UsageTable>>
//     {
//         template<typename T, auto NormalizedUsage = detail::normalize_usage(UsageTable, tree_shape<T>)>
//         constexpr decltype(auto) operator()(T&& t)const
//         {
//             //clang bug(clang 18.1): https://gcc.godbolt.org/z/8KfEo94Kv
//             //constexpr auto normalized_usage_table =detail::normalize_usage(UsageTable, tree_shape<T>);

//             if constexpr(requires{ FWD(t).template simplified_data<NormalizedUsage>(custom_t{}); })
//             {
//                 return FWD(t).template simplified_data<NormalizedUsage>(custom_t{});
//             }
//             else if constexpr(requires{ simplified_data<NormalizedUsage>(FWD(t), custom_t{}); })
//             {
//                 return simplified_data<NormalizedUsage>(FWD(t), custom_t{});
//             }
//             else if constexpr(requires{ FWD(t).template simplified_data<NormalizedUsage>(); })
//             {
//                 return FWD(t).template simplified_data<NormalizedUsage>();
//             }
//             else if constexpr(requires{ simplified_data<NormalizedUsage>(FWD(t)); })
//             {
//                 return simplified_data<NormalizedUsage>(FWD(t));
//             }
//             else if constexpr(simple<T, NormalizedUsage>)
//             {
//                 return FWD(t) | child<>;
//             }
//             else return [&]<size_t...I>(std::index_sequence<I...>)
//             {
//                 return rzx::tuple<decltype(FWD(t) | child<I> | rzx::simplified_data<NormalizedUsage | child<I>>)...>
//                 {
//                     FWD(t) | child<I> | rzx::simplified_data<NormalizedUsage | child<I>>...
//                 };
//             }(std::make_index_sequence<child_count<T>>{});
//         }
//     };
// }

// namespace rzx 
// {  
//     namespace detail::simplified_layout_t_ns
//     {
//         template<auto UsageTable>
//         void get_simplified_layout();
        
//         template<class T, auto UsageTable>
//         constexpr auto simplified_layout()
//         {
//             if constexpr(requires{ get_simplified_layout<UsageTable>(type_tag<T>{}); })
//             {
//                 return get_simplified_layout<UsageTable>(type_tag<T>{});
//             }
//             else if constexpr(simple<T, UsageTable>)
//             {
//                 return indexes_of_whole;
//             }
//             else return [&]<size_t...I>(std::index_sequence<I...>)
//             {
//                 return rzx::make_tuple(
//                     detail::layout_add_prefix(simplified_layout<child_type<T, I>, UsageTable | child<I>>(), array{ I })...
//                 );
//             }(std::make_index_sequence<child_count<T>>{});
//         };
//     }

//     template<class T, auto UsageTable = usage_t::repeatedly>
//     inline constexpr auto simplified_layout = detail::simplified_layout_t_ns::simplified_layout<T, detail::normalize_usage(UsageTable, tree_shape<T>)>();
// }

#include "macro_undef.hpp"
#endif