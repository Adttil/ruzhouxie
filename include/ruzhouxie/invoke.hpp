#ifndef RUZHOUXIE_INVOKE_HPP
#define RUZHOUXIE_INVOKE_HPP

#include <functional>

#include "astrict.hpp"
#include "general.hpp"
#include "relayout.hpp"
#include "simplify.hpp"
#include "view_interface.hpp"

#include "macro_define.hpp"

namespace rzx::detail
{
    template<auto OperationTable>
    constexpr auto simplify_operation_table()
    {
        if constexpr(terminal<decltype(OperationTable)>)
        {
            return OperationTable;
        }
        else return []<size_t...I>(std::index_sequence<I...>)
        {
            constexpr auto result = rzx::make_tuple(simplify_operation_table<OperationTable | child<I>>()...);
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

    template<class UsageTable>
    constexpr auto set_usage_table_all_repeat(UsageTable& usage_table)
    {
        if constexpr(terminal<UsageTable>)
        {
            static_assert(std::same_as<UsageTable, usage_t>);
            if(usage_table == usage_t::once)
            {
                usage_table = usage_t::repeatedly;
            }
        }
        else [&]<size_t...I>(std::index_sequence<I...>)
        {
            (..., set_usage_table_all_repeat(usage_table | child<I>)); 
        }(std::make_index_sequence<child_count<UsageTable>>{});
    }

    template<class UsageTable>
    constexpr auto usage_table_all_repeat(UsageTable usage_table)
    {
        set_usage_table_all_repeat(usage_table);
        return usage_table;
    }

    template<class ArgTable, class OperationTable>
    constexpr decltype(auto) apply_operate(ArgTable&& arg_table, OperationTable = {})
    {
        if constexpr(terminal<OperationTable>)
        {
            return OperationTable{}(FWD(arg_table));
            // if constexpr(std::same_as<OperationTable, no_operation_t>)
            // {
            //     return ArgTable{ FWD(arg_table) };
            // }
            // else
            // {
            //     return OperationTable{}(FWD(arg_table));
            // }
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return rzx::tuple<decltype(apply_operate(arg_table | child<I>, std::decay_t<child_type<OperationTable, I>>{}))...>
            {
                apply_operate(arg_table | child<I>, std::decay_t<child_type<OperationTable, I>>{})...
            };
        }(std::make_index_sequence<child_count<OperationTable>>{});
    }
}

namespace rzx 
{
    namespace detail
    {
        template<class ArgTable, class OperationTable>
        struct operate_view_storage
        {
            RUZHOUXIE(no_unique_address) ArgTable       arg_table;
            RUZHOUXIE(no_unique_address) OperationTable operation_table;
        };
    }

    template<class ArgTable, class OperationTable>
    struct operate_view;
    
    template<class ArgTable, class OperationTable>
    operate_view(ArgTable, OperationTable) -> operate_view<ArgTable, OperationTable>;
    
    namespace detail
    {
        struct operate_t : adaptor<operate_t>
        {
            template<class ArgTable, class OperationTable>
            constexpr auto result(ArgTable&& arg_table, OperationTable)const
            {
                constexpr auto op_table = detail::simplify_operation_table<OperationTable{}>();
                if constexpr(rzx::equal(op_table, no_operation))
                {
                    return view<unwrap_t<ArgTable>>{ unwrap(FWD(arg_table)) };
                }
                else
                {
                    return operate_view<unwrap_t<ArgTable>, std::remove_const_t<decltype(op_table)>>{
                        unwrap(FWD(arg_table))
                    };
                }
            }
        };
    }
    
    inline constexpr detail::operate_t operate{};

}

namespace rzx {

    template<class ArgTable, class OperationTable>
    struct operate_view : detail::operate_view_storage<ArgTable, OperationTable>, view_interface<operate_view<ArgTable, OperationTable>>
    {
        template<size_t I, class Self>
        constexpr decltype(auto) get(this Self&& self)
        {
            if constexpr(I >= child_count<OperationTable>)
            {
                return end();
            }
            else if constexpr(terminal<child_type<OperationTable, I>>)
            {
                //return detail::apply_operate(FWD(self, arg_table) | child<I>, OperationTable{} | child<I>);
                return std::decay_t<child_type<OperationTable, I>>{}(FWD(self, arg_table) | child<I>);
            }
            else
            {
                return operate_view<decltype(FWD(self, arg_table) | child<I>), std::decay_t<child_type<OperationTable, I>>>
                {
                    FWD(self, arg_table) | child<I>
                };
            }
        }

        template<auto UsageTable, typename Self>
        constexpr auto simplifier(this Self&& self)
        {
            struct simplifier_t
            {
                Self&& self;

                constexpr auto base_simplifer()const
                {
                    if constexpr(std::is_reference_v<ArgTable>)
                    {
                        return FWD(self, arg_table) | rzx::simplifier<detail::usage_table_all_repeat(UsageTable)>;
                    }
                    else
                    {
                        return FWD(self, arg_table) | refer | rzx::simplifier<detail::usage_table_all_repeat(UsageTable)>;
                    }
                }

                static consteval auto layout()
                {
                    return indexes_of_whole;
                    //return base_simplifer_t::layout();
                }

                static consteval auto operation_table()
                {
                    //static_assert(equal(base_simplifer_t::operation_table(), no_operation));
                    return no_operation;
                    //return OperationTable{};
                }

                constexpr decltype(auto) data()const
                {
                    using base_simplifer_t = decltype(base_simplifer());
                    return detail::apply_operate(base_simplifer().data() | relayout<base_simplifer_t::layout()>, OperationTable{});
                }
            };

            return simplifier_t{ FWD(self) };
        }
    };
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
                auto simplifier = FWD(t) | rzx::simplifier<>;

                using data_shape_t = tree_shape_t<decltype(simplifier.data())>;
                constexpr auto layout = simplifier.layout();
                constexpr auto nlayout = detail::normalize_layout2<layout, data_shape_t>();
                constexpr auto stricture_table = detail::stricture_table_for_sequence<nlayout, data_shape_t>();

                using simplified_t = decltype(simplifier.data() | relayout<layout>);
                constexpr auto sstricture_table = detail::simplify_stricture_table<stricture_table>(tree_shape<simplified_t>);

                return simplifier.data() 
                    | relayout<layout> 
                    | astrict<sstricture_table> 
                    | operate(simplifier.operation_table());
            }

            template<terminal T>
            constexpr tuple<> operator()(T&& t)const
            {
                return {};
            }
        };
    }

    inline constexpr detail::sequence_t sequence{};

    inline constexpr auto inverse_sequence = inverse | sequence;

    namespace detail
    {
        struct seperate_t : adaptor_closure<seperate_t>
        {
            template<branched T>
            constexpr decltype(auto) operator()(T&& t)const
            {
                auto simplifier = FWD(t) | rzx::simplifier<>;

                using data_shape_t = tree_shape_t<decltype(simplifier.data())>;
                constexpr auto layout = simplifier.layout();
                constexpr auto nlayout = detail::normalize_layout2<layout, data_shape_t>();
                constexpr auto stricture_table = detail::stricture_table_for_children<nlayout, data_shape_t>();

                using simplified_t = decltype(simplifier.data() | relayout<layout>);
                constexpr auto sstricture_table = detail::simplify_stricture_table<stricture_table>(tree_shape<simplified_t>);

                return simplifier.data()
                 | relayout<layout>
                 | astrict<sstricture_table>
                 | operate(simplifier.operation_table());
                 ;
            }

            template<terminal T>
            constexpr tuple<> operator()(T&& t)const
            {
                return {};
            }
        };
    }

    inline constexpr detail::seperate_t seperate{};

    struct apply_invoke_t
    {
        template<class Args>
        constexpr decltype(auto) operator()(Args&& args)const noexcept
        {
            return [&]<size_t...I>(std::index_sequence<I...>) -> decltype(auto)
            {
                auto&& seperate_args = FWD(args) | refer | seperate;
                return (FWD(seperate_args) | child<0>)(FWD(seperate_args) | child<I + 1uz> ...);
                //return std::invoke(FWD(seperate_args) | child<I> ...);
            }(std::make_index_sequence<child_count<Args> - 1>{});
        }

        friend constexpr bool operator==(apply_invoke_t, apply_invoke_t) = default;
    };

    inline constexpr apply_invoke_t apply_invoke{};
}

namespace rzx {
    namespace detail
    {
        struct transform_t : adaptor<transform_t>
        {
            template<class ArgTable, class Fn>
            constexpr decltype(auto) result(ArgTable&& arg_table, Fn&& fn)const
            {
                constexpr size_t n = child_count<ArgTable>;
                return zip(FWD(fn) | repeat<n>, FWD(arg_table)) 
                    | operate(make_tree_of_same_value(apply_invoke, tree_shape<array<size_t, n>>));
            }
        };
    }
    
    inline constexpr detail::transform_t transform{};
    
    namespace detail
    {
        struct zip_transform_t
        {
            template<class Fn, class Args, class...Rest>
            constexpr decltype(auto) operator()(Fn&& fn, Args&& args, Rest&&...rest)const
            {
                constexpr size_t n = child_count<Args>;
                return zip(FWD(fn) | repeat<n>, FWD(args), FWD(rest)...) 
                    | operate(make_tree_of_same_value(apply_invoke, tree_shape<array<size_t, n>>));
            }
        };
    }

    inline constexpr detail::zip_transform_t zip_transform{};

    
}

// namespace rzx::detail
// {
//     struct no_cache_t{};

//     //UsageTable shohuld be normalized.
//     template<auto UsageTable, class ArgTable, class FnTable>
//     constexpr decltype(auto) get_cache(ArgTable&& arg_table, FnTable&& fn_table)
//     {
//         if constexpr(requires{ FWD(fn_table)(FWD(arg_table)); })
//         {
//             static_assert(std::same_as<decltype(UsageTable), usage_t>, "Invalid usage table.");
//             if constexpr(UsageTable == usage_t::repeatedly)
//             {
//                 return FWD(fn_table)(FWD(arg_table));
//             }
//             else
//             {
//                 return no_cache_t{};
//             }
//         }
//         else return[&]<size_t...I>(std::index_sequence<I...>)
//         {
//             return rzx::tuple<decltype(get_cache<UsageTable | child<I>>(FWD(arg_table) | child<I>, FWD(fn_table) | child<I>))...>
//             { 
//                 get_cache<UsageTable | child<I>>(FWD(arg_table) | child<I>, FWD(fn_table) | child<I>)...
//             };
//         }(std::make_index_sequence<child_count<FnTable>>{});
//     }
// }

// namespace rzx 
// {
//     namespace detail
//     {
//         template<class ArgTable, class FnTable>
//         struct invoke_view_storage
//         {
//             RUZHOUXIE(no_unique_address) ArgTable arg_table;
//             RUZHOUXIE(no_unique_address) FnTable  fn_table;
//         };
//     }

//     template<class ArgTable, class FnTable>
//     struct invoke_view : detail::invoke_view_storage<ArgTable, FnTable>, view_interface<invoke_view<ArgTable, FnTable>>
//     {
//         // template<typename Self>
//         // constexpr decltype(auto) self(this Self&& self)
//         // {
//         //     if constexpr(std::is_object_v<Self> && std::is_reference_v<ArgTable> && std::is_reference_v<FnTable>)
//         //     {
//         //         return invoke_view{ FWD(self) };
//         //     }
//         //     else
//         //     {
//         //         return FWD(self);
//         //     }
//         // }

//         template<size_t I, class Self>
//         constexpr decltype(auto) get(this Self&& self)
//         {
//             if constexpr(I >= child_count<FnTable>)
//             {
//                 return end();
//             }
//             else if constexpr(requires{ (FWD(self, fn_table) | child<I>)(FWD(self, arg_table) | child<I>); })
//             {
//                 return (FWD(self, fn_table) | child<I>)(FWD(self, arg_table) | child<I>);
//             }
//             else
//             {
//                 return invoke_view<decltype(FWD(self, fn_table) | child<I>), decltype(FWD(self, arg_table) | child<I>)>
//                 {
//                     FWD(self, fn_table) | child<I>,
//                     FWD(self, arg_table) | child<I>
//                 };
//             }
//         }

//     //private:
//         template<size_t I, auto UsageTable, typename Self>
//         constexpr decltype(auto) child_data(this Self&& self)
//         {
//             if constexpr(requires{ (FWD(self, fn_table) | child<I>)(FWD(self, arg_table) | child<I>); })
//             {
//                 return (FWD(self, fn_table) | child<I>)(FWD(self, arg_table) | child<I>);
//             }
//             else
//             {
//                 return FWD(self) | child<I> | rzx::simplified_data<UsageTable>;
//             }
//         }

//         template<class Data>
//         struct simplify_result_type
//         {
//             Data data;

//             static constexpr decltype(auto) init_cache(Data&& data)
//             {
//                 return [&]<size_t...I>(std::index_sequence<I...>)
//                 {
//                     return tuple<std::decay_t<decltype(FWD(data).template child_data<I, usage_t::repeatedly>())>...>{
//                         FWD(data).template child_data<I, usage_t::repeatedly>()...
//                     };
//                 }(std::make_index_sequence<child_count<Data>>{});
//             }

//             decltype(init_cache(FWD(data))) cache = init_cache(FWD(data));
//         };
//     public:
//         template<auto UsageTable, typename Self>
//         constexpr decltype(auto) get_data(this Self&& self)
//         {
//             using arg_t = decltype(FWD(self, arg_table) | rzx::simplify<UsageTable>);

//             return invoke_view<arg_t, decltype(FWD(self, fn_table))>{ 
//                 FWD(self, arg_table) | rzx::simplify<UsageTable>,
//                 FWD(self, fn_table)
//             };
//         }

//         template<auto UsageTable, typename Self>
//         constexpr auto simplifier(this Self&& self)
//         {
//             using base_t = decltype(FWD(self).template get_data<UsageTable>());

//             struct simplifier_t
//             {
//                 Self&& self;

//                 static constexpr auto layout(){ return array{ 1uz }; }

//                 constexpr decltype(auto) data()
//                 { 
//                     return simplify_result_type<base_t>
//                     {
//                         FWD(self).template get_data<UsageTable>()
//                     };
//                 }
//             };

//             return simplifier_t{ FWD(self) };
//         }

//         // template<auto UsageTable, typename Self>
//         // constexpr decltype(auto) simplified_data(this Self&& self)
//         // {
//         //     using base_t = decltype(FWD(self).template get_data<UsageTable>());

//         //     return simplify_result_type<base_t>
//         //     {
//         //         FWD(self).template get_data<UsageTable>()
//         //     };

//         //     // return [&]<size_t...I>(std::index_sequence<I...>)
//         //     // {
//         //     //     return tuple<std::decay_t<decltype(FWD(self).template child_data<I, UsageTable>())>...>{
//         //     //         FWD(self).template child_data<I, UsageTable>()...
//         //     //     };
//         //     // }(std::make_index_sequence<child_count<Self>>{});
//         //     //return FWD(self).self();
//         // }

//         // template<auto UsageTable, derived_from<invoke_view> Self>
//         // friend constexpr decltype(auto) get_simplified_layout(type_tag<Self>)
//         // {
//         //     return array{ 1uz };

//         //     //return indexes_of_whole;
//         // }
//     };

//     template<class ArgTable, class FnTable>
//     invoke_view(ArgTable, FnTable) -> invoke_view<ArgTable, FnTable>;

//     namespace detail
//     {
//         struct invoke_t : adaptor<invoke_t>
//         {
//             template<class ArgTable, class FnTable>
//             constexpr decltype(auto) result(ArgTable&& arg_table, FnTable&& fn_table)const
//             {
//                 return invoke_view<unwrap_t<ArgTable>, unwrap_t<FnTable>>{ unwrap(FWD(arg_table)), unwrap(FWD(fn_table)) };
//             }
//         };
//     }
    
//     inline constexpr detail::invoke_t invoke{};
    
//     namespace detail
//     {
//         struct transform_t : adaptor<transform_t>
//         {
//             template<class ArgTable, class Fn>
//             constexpr decltype(auto) result(ArgTable&& arg_table, Fn&& fn)const
//             {
//                 constexpr size_t n = child_count<ArgTable>;
//                 return invoke_view<unwrap_t<ArgTable>, decltype(FWD(fn) | repeat<n>)>{ unwrap(FWD(arg_table)), FWD(fn) | repeat<n> };
//             }
//         };
//     }
    
//     inline constexpr detail::transform_t transform{};
    
//     namespace detail
//     {
//         struct zip_transform_t
//         {
//             template<class Fn>
//             struct closure_type
//             {
//                 Fn fn;
                
//                 constexpr decltype(auto) operator()(this auto&& self, auto&& args)
//                 {
//                     return rzx::apply(FWD(args), FWD(self, fn));
//                 }
//             };

//             template<class Fn, class...ArgTables>
//             constexpr decltype(auto) operator()(Fn&& fn, ArgTables&&...arg_tables)const
//             {
//                 struct capture_t{ Fn fn; };
//                 return tuple<ArgTables...>{ FWD(arg_tables)... }
//                        | transpose<> 
//                        | transform(closure_type<Fn>{ FWD(fn) });
//             }
//         };
//     }

//     inline constexpr detail::zip_transform_t zip_transform{};

    
// }

#include "macro_undef.hpp"
#endif