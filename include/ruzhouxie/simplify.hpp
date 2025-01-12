#ifndef RUZHOUXIE_SIMPLIFY_HPP
#define RUZHOUXIE_SIMPLIFY_HPP

#include "child.hpp"
#include "view_interface.hpp"
#include "general.hpp"

#include "macro_define.hpp"

namespace rzx 
{  
    enum class stricture_t
    {
        none,
        readonly
    };

    enum class operation_t
    {
        none,

    };

    struct no_operation_t
    {
        template<class T>
        constexpr T&& operator()(T&& t)const noexcept
        {
            return FWD(t);
        }

        friend constexpr bool operator==(no_operation_t, no_operation_t) = default;
    };

    inline constexpr no_operation_t no_operation{};

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
    template<auto UnfoldedLayout, typename U, typename R>
    constexpr void inverse_apply_layout_on_usage_at(const U& usage, R& result)
    {
        if constexpr(indexical<decltype(UnfoldedLayout)>)
        {
            auto&& result_usage = result | child<UnfoldedLayout>;
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
            (..., inverse_apply_layout_on_usage_at<UnfoldedLayout | child<I>>(usage | child<I>, result));
        }(std::make_index_sequence<child_count<decltype(UnfoldedLayout)>>{});
    }

    template<auto UnfoldedLayout, typename U, typename S>
    constexpr auto inverse_apply_layout_on_usage(const U& usage, const S& shape)
    {
        auto result = make_tree_of_same_value(usage_t::discard, shape);
        inverse_apply_layout_on_usage_at<UnfoldedLayout>(usage, result);
        return result;
    }

    template<auto FoldedLayout, typename U, typename S>
    constexpr auto inverse_apply_layout_on_usage_table(const U& usage, S shape = {})
    {

    }

    template<typename Tag, typename Shape, typename TagTable>
    constexpr auto unfold_tag_table(const TagTable& tag_table, Shape shape = {})
    {
        if constexpr(terminal<Shape>)
        {
            static_assert(std::same_as<TagTable, Tag>, "Invalid tag table.");
            return tag_table;
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            if constexpr(branched<TagTable>)
            {
                constexpr size_t n_pad = child_count<Shape> > child_count<TagTable> ? child_count<Shape> - child_count<TagTable> : 0uz;
                constexpr auto pad = array<Tag, n_pad>{};
                const auto padded_tag_table = concat_to_tuple(tag_table, pad);
                return rzx::make_tuple(unfold_tag_table<Tag>(padded_tag_table | child<I>, shape | child<I>)...);
            }
            else
            {
                static_assert(std::same_as<TagTable, Tag>, "Invalid tag table.");
                return rzx::make_tuple(unfold_tag_table<Tag>(tag_table, shape | child<I>)...);
            }
        }(std::make_index_sequence<child_count<Shape>>{});
    }

    template<typename Shape, typename UsageTable>
    constexpr auto unfold_usage_table(const UsageTable& usage_table, Shape shape = {})
    {
        return unfold_tag_table<usage_t>(usage_table, shape);
    }

    template<typename Shape, typename StrictureTable>
    constexpr auto unfold_stricture_table(const StrictureTable& stricture_table, Shape shape = {})
    {
        return unfold_tag_table<stricture_t>(stricture_table, shape);
    }

    template<auto OperationTable>
    constexpr auto fold_operation_table()
    {
        if constexpr(terminal<decltype(OperationTable)>)
        {
            return OperationTable;
        }
        else return []<size_t...I>(std::index_sequence<I...>)
        {
            constexpr auto result = rzx::make_tuple(fold_operation_table<OperationTable | child<I>>()...);
            if constexpr((... && rzx::equal(result | child<I>, no_operation)))
            {
                return no_operation;
            }
            else
            {
                return result;
            }
        }(std::make_index_sequence<child_count<decltype(OperationTable)>>{});
    }

    // template<auto OperationTable>
    // constexpr auto unfold_operation_table()
    // {
    //     if constexpr(terminal<decltype(OperationTable)>)
    //     {
    //         return OperationTable;
    //     }
    //     else return []<size_t...I>(std::index_sequence<I...>)
    //     {
    //         constexpr auto result = rzx::make_tuple(fold_operation_table<OperationTable | child<I>>()...);
    //         if constexpr((... && rzx::equal(result | child<I>, no_operation)))
    //         {
    //             return no_operation;
    //         }
    //         else
    //         {
    //             return result;
    //         }
    //     }(std::make_index_sequence<child_count<decltype(OperationTable)>>{});
    // }
}

namespace rzx 
{  
    namespace detail::get_simplifier_t_ns
    {
        template<auto UsageTable>
        struct get_simplifier_t;
        
        template<auto UsageTable>
        void simplifier();
        
        template<class T, auto UnfoldedUsageTable>
        constexpr bool is_simple()
        {
            constexpr bool no_custom = not bool
            {
                requires{ std::declval<T>().template simplifier<UnfoldedUsageTable>(custom_t{}); }
                ||
                requires{ simplifier<UnfoldedUsageTable>(std::declval<T>(), custom_t{}); }
                ||
                requires{ std::declval<T>().template simplifier<UnfoldedUsageTable>(); }
                ||
                requires{ simplifier<UnfoldedUsageTable>(std::declval<T>()); }
            };
            return [&]<size_t...I>(std::index_sequence<I...>)
            {
                return (no_custom && ... && is_simple<child_type<T, I>, UnfoldedUsageTable | child<I>>());
            }(std::make_index_sequence<child_count<T>>{});
        }
    }

    template<auto UsageTable = usage_t::repeatedly>
    inline constexpr detail::get_simplifier_t_ns::get_simplifier_t<UsageTable> simplifier{};

    template<class T, auto UsageTable = usage_t::repeatedly>
    concept simple = detail::get_simplifier_t_ns::is_simple<T, detail::unfold_usage_table(UsageTable, tree_shape<T>)>();

    template<class T>
    struct simple_simplifier
    {
        T&& t;

        static constexpr auto operation_table(){ return no_operation; }

        static constexpr auto stricture_table(){ return stricture_t::none; }

        static constexpr auto layout(){ return indexes_of_whole; }

        constexpr decltype(auto) data()const
        {
            //???
            if constexpr(std::is_object_v<T>)
            {
                return FWD(t); 
            } 
            else
            {
                return FWD(t) | refer;
            }
        }
    };

    template<class T, auto UnfoldedUsageTable>
    struct trivial_simplifier
    {
        T&& t;

        static constexpr auto operation_table()
        { 
            return []<size_t...I>(std::index_sequence<I...>)
            {
                return rzx::make_tuple(
                    decltype(std::declval<child_type<T, I>>() | simplifier<UnfoldedUsageTable | child<I>>)::operation_table()...
                );
            }(std::make_index_sequence<child_count<T>>{});
        }

        static constexpr auto stricture_table()
        {
            return []<size_t...I>(std::index_sequence<I...>)
            {
                return rzx::make_tuple(
                    decltype(std::declval<child_type<T, I>>() | simplifier<UnfoldedUsageTable | child<I>>)::stricture_table()...
                );
            }(std::make_index_sequence<child_count<T>>{});
        }

        static constexpr auto layout()
        {
            return []<size_t...I>(std::index_sequence<I...>)
            {
                return rzx::make_tuple(
                    detail::layout_add_prefix(decltype(std::declval<child_type<T, I>>() | simplifier<UnfoldedUsageTable | child<I>>)::layout(), array{ I })...
                );
            }(std::make_index_sequence<child_count<T>>{});
        }

        template<class V>
        static constexpr unwrap_t<V> unwrap_copy(V&& v)
        {
            return unwrap_t<V>{ unwrap(FWD(v)) };
        }

        template<size_t I>
        constexpr decltype(auto) child_data()const
        {
            if constexpr((std::is_object_v<T> && std::is_object_v<rzx::detail::tuple_element_t_by_child<I, T>>) 
                        || std::is_object_v<child_type<T, I>>)
            {
                // return unwrap_t<decltype((FWD(t) | child<I> | simplifier<UsageTable | child<I>>).data())>
                // {
                //     unwrap((FWD(t) | child<I> | simplifier<UsageTable | child<I>>).data())
                // };

                auto simplifier = FWD(t) | child<I> | rzx::simplifier<UnfoldedUsageTable | child<I>>;

                //https://gcc.godbolt.org/z/rM983Yh9f
                return unwrap_t<decltype(simplifier.data())>
                (
                    simplifier.data()
                );
            }
            else
            {
                auto simplifier = FWD(t) | child<I> | refer | rzx::simplifier<UnfoldedUsageTable | child<I>>;
                return unwrap_t<decltype(simplifier.data())>
                (
                    simplifier.data()
                );
            }
        }

        constexpr auto data()const
        {
            return [&]<size_t...I>(std::index_sequence<I...>)
            {
                return rzx::tuple<decltype(child_data<I>())...>
                {
                    child_data<I>()...
                };
            }(std::make_index_sequence<child_count<T>>{});
        }
    };

    

    template<class S>
    struct ref_view_simplifier
    {
        S base_simplifer;

        static constexpr auto operation_table(){ return S::operation_table(); }

        static constexpr auto stricture_table(){ return S::stricture_table(); }

        static constexpr auto layout(){ return S::layout(); }

        constexpr decltype(auto) data()const
        {
            if constexpr(std::is_rvalue_reference_v<decltype(base_simplifer.data())>)
            {
                return base_simplifer.data() | refer; 
            } 
            else
            {
                return base_simplifer.data();
            }
        }
    };

    template<auto UsageTable>
    struct detail::get_simplifier_t_ns::get_simplifier_t : adaptor_closure<get_simplifier_t<UsageTable>>
    {
        template<typename T>
        constexpr auto operator()(T&& t)const
        {
            //clang bug(clang 18.1): https://gcc.godbolt.org/z/8KfEo94Kv
            constexpr auto unfolded_usage_table = detail::unfold_usage_table(UsageTable, tree_shape<T>);

            if constexpr(std::is_rvalue_reference_v<unwrap_t<T>>)
            {
                return ref_view_simplifier{ impl<unwrap_t<T>, unfolded_usage_table>(unwrap(FWD(t))) };
            }
            else
            {
                return impl<unwrap_t<T>, unfolded_usage_table>(unwrap(FWD(t)));
            }
        }

        template<typename T, auto UnfoldedUsageTable>
        constexpr auto impl(T&& t)const
        {
            //clang bug(clang 18.1): https://gcc.godbolt.org/z/8KfEo94Kv
            //constexpr auto unfolded_usage_table = detail::unfold_usage_table(UsageTable, tree_shape<T>);

            if constexpr(requires{ FWD(t).template simplifier<UnfoldedUsageTable>(custom_t{}); })
            {
                return FWD(t).template simplifier<UnfoldedUsageTable>(custom_t{});
            }
            else if constexpr(requires{ simplifier<UnfoldedUsageTable>(FWD(t), custom_t{}); })
            {
                return simplifier<UnfoldedUsageTable>(FWD(t), custom_t{});
            }
            else if constexpr(requires{ FWD(t).template simplifier<UnfoldedUsageTable>(); })
            {
                return FWD(t).template simplifier<UnfoldedUsageTable>();
            }
            else if constexpr(requires{ simplifier<UnfoldedUsageTable>(FWD(t)); })
            {
                return simplifier<UnfoldedUsageTable>(FWD(t));
            }
            else if constexpr(simple<T, UnfoldedUsageTable>)
            {                
                return simple_simplifier<T>{ FWD(t) };
            }
            else
            { 
                return trivial_simplifier<T, UnfoldedUsageTable>{ FWD(t) };
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

    template<class T, auto UsageTable = usage_t::repeatedly>
    inline constexpr auto simplified_stricture_table = decltype(std::declval<T>() | simplifier<UsageTable>)::stricture_table();

    template<class T, auto UsageTable = usage_t::repeatedly>
    inline constexpr auto simplified_operation_table = decltype(std::declval<T>() | simplifier<UsageTable>)::operation_table();
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