#ifndef RUZHOUXIE_INVOKE_HPP
#define RUZHOUXIE_INVOKE_HPP

#include "general.hpp"
#include "relayout.hpp"
#include "simplify.hpp"
#include "view_interface.hpp"

#include "macro_define.hpp"

namespace rzx::detail
{
    struct no_cache_t{};

    //UsageTable shohuld be normalized.
    template<auto UsageTable, class ArgTable, class FnTable>
    constexpr decltype(auto) get_cache(ArgTable&& arg_table, FnTable&& fn_table)
    {
        if constexpr(requires{ FWD(fn_table)(FWD(arg_table)); })
        {
            static_assert(std::same_as<decltype(UsageTable), usage_t>, "Invalid usage table.");
            if constexpr(UsageTable == usage_t::repeatedly)
            {
                return FWD(fn_table)(FWD(arg_table));
            }
            else
            {
                return no_cache_t{};
            }
        }
        else return[&]<size_t...I>(std::index_sequence<I...>)
        {
            return rzx::tuple<decltype(get_cache<UsageTable | child<I>>(FWD(arg_table) | child<I>, FWD(fn_table) | child<I>))...>
            { 
                get_cache<UsageTable | child<I>>(FWD(arg_table) | child<I>, FWD(fn_table) | child<I>)...
            };
        }(std::make_index_sequence<child_count<FnTable>>{});
    }
}

namespace rzx 
{
    namespace detail
    {
        template<class ArgTable, class FnTable>
        struct invoke_view_storage
        {
            RUZHOUXIE(no_unique_address) ArgTable arg_table;
            RUZHOUXIE(no_unique_address) FnTable  fn_table;
        };
    }

    template<class ArgTable, class FnTable>
    struct invoke_view : detail::invoke_view_storage<ArgTable, FnTable>, view_interface<invoke_view<ArgTable, FnTable>>
    {
        // template<typename Self>
        // constexpr decltype(auto) self(this Self&& self)
        // {
        //     if constexpr(std::is_object_v<Self> && std::is_reference_v<ArgTable> && std::is_reference_v<FnTable>)
        //     {
        //         return invoke_view{ FWD(self) };
        //     }
        //     else
        //     {
        //         return FWD(self);
        //     }
        // }

        template<size_t I, class Self>
        constexpr decltype(auto) get(this Self&& self)
        {
            if constexpr(I >= child_count<FnTable>)
            {
                return end();
            }
            else if constexpr(requires{ (FWD(self, fn_table) | child<I>)(FWD(self, arg_table) | child<I>); })
            {
                return (FWD(self, fn_table) | child<I>)(FWD(self, arg_table) | child<I>);
            }
            else
            {
                return invoke_view<decltype(FWD(self, fn_table) | child<I>), decltype(FWD(self, arg_table) | child<I>)>
                {
                    FWD(self, fn_table) | child<I>,
                    FWD(self, arg_table) | child<I>
                };
            }
        }

    //private:
        template<size_t I, auto UsageTable, typename Self>
        constexpr decltype(auto) child_data(this Self&& self)
        {
            if constexpr(requires{ (FWD(self, fn_table) | child<I>)(FWD(self, arg_table) | child<I>); })
            {
                return (FWD(self, fn_table) | child<I>)(FWD(self, arg_table) | child<I>);
            }
            else
            {
                return (FWD(self) | child<I> | rzx::get_simplifier<UsageTable>).data();
                //return FWD(self) | child<I> | rzx::simplified_data<UsageTable>;
            }
        }

        template<class Data>
        struct simplify_result_type
        {
            Data data;

            static constexpr decltype(auto) init_cache(Data&& data)
            {
                return [&]<size_t...I>(std::index_sequence<I...>)
                {
                    return tuple<std::decay_t<decltype(FWD(data).template child_data<I, usage_t::repeatedly>())>...>{
                        FWD(data).template child_data<I, usage_t::repeatedly>()...
                    };
                }(std::make_index_sequence<child_count<Data>>{});
            }

            decltype(init_cache(FWD(data))) cache = init_cache(FWD(data));
        };
    public:
        template<auto UsageTable, typename Self>
        constexpr decltype(auto) get_data(this Self&& self)
        {
            using arg_t = decltype(FWD(self, arg_table) | rzx::simplify<UsageTable>);

            return invoke_view<arg_t, decltype(FWD(self, fn_table))>{ 
                FWD(self, arg_table) | rzx::simplify<UsageTable>,
                FWD(self, fn_table)
            };
        }

        template<auto UsageTable, typename Self>
        constexpr auto simplifier(this Self&& self)
        {
            using base_t = decltype(FWD(self).template get_data<UsageTable>());

            struct simplifier_t
            {
                Self&& self;

                static constexpr auto layout(){ return array{ 1uz }; }

                constexpr decltype(auto) data()
                { 
                    return simplify_result_type<base_t>
                    {
                        FWD(self).template get_data<UsageTable>()
                    };
                }
            };

            return simplifier_t{ FWD(self) };
        }

        template<auto UsageTable, typename Self>
        constexpr decltype(auto) simplified_data(this Self&& self)
        {
            using base_t = decltype(FWD(self).template get_data<UsageTable>());

            return simplify_result_type<base_t>
            {
                FWD(self).template get_data<UsageTable>()
            };

            // return [&]<size_t...I>(std::index_sequence<I...>)
            // {
            //     return tuple<std::decay_t<decltype(FWD(self).template child_data<I, UsageTable>())>...>{
            //         FWD(self).template child_data<I, UsageTable>()...
            //     };
            // }(std::make_index_sequence<child_count<Self>>{});
            //return FWD(self).self();
        }

        template<auto UsageTable, derived_from<invoke_view> Self>
        friend constexpr decltype(auto) get_simplified_layout(type_tag<Self>)
        {
            return array{ 1uz };

            //return indexes_of_whole;
        }
    };

    template<class ArgTable, class FnTable>
    invoke_view(ArgTable, FnTable) -> invoke_view<ArgTable, FnTable>;

    namespace detail
    {
        struct invoke_t : adaptor<invoke_t>
        {
            template<class ArgTable, class FnTable>
            constexpr decltype(auto) result(ArgTable&& arg_table, FnTable&& fn_table)const
            {
                return invoke_view<ArgTable, std::decay_t<FnTable>>{ FWD(arg_table), FWD(fn_table) };
            }
        };
    }
    
    inline constexpr detail::invoke_t invoke{};
    
    namespace detail
    {
        struct transform_t : adaptor<transform_t>
        {
            template<class ArgTable, class Fn>
            constexpr decltype(auto) result(ArgTable&& arg_table, Fn&& fn)const
            {
                constexpr size_t n = child_count<ArgTable>;
                return invoke_view<ArgTable, decltype(FWD(fn) | repeat<n>)>{ FWD(arg_table), FWD(fn) | repeat<n> };
            }
        };
    }
    
    inline constexpr detail::transform_t transform{};
    
    namespace detail
    {
        struct zip_transform_t
        {
            template<class Fn>
            struct closure_type
            {
                Fn fn;
                
                constexpr decltype(auto) operator()(this auto&& self, auto&& args)
                {
                    return rzx::apply(FWD(self, fn), FWD(args));
                }
            };

            template<class Fn, class...ArgTables>
            constexpr decltype(auto) operator()(Fn&& fn, ArgTables&&...arg_tables)const
            {
                struct capture_t{ Fn fn; };
                return tuple<ArgTables...>{ FWD(arg_tables)... }
                       | transpose<> 
                       | transform(closure_type<Fn>{ FWD(fn) });
            }
        };
    }

    inline constexpr detail::zip_transform_t zip_transform{};

    
}

#include "macro_undef.hpp"
#endif