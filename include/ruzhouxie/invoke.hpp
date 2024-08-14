#ifndef RUZHOUXIE_INVOKE_HPP
#define RUZHOUXIE_INVOKE_HPP

#include "general.hpp"
#include "relayout.hpp"
#include "simplify.hpp"
#include "view_interface.hpp"

#include "macro_define.hpp"

namespace rzx 
{
    namespace detail
    {
        template<class FnTable, class ArgTable>
        struct invoke_view_storage
        {
            RUZHOUXIE(no_unique_address) FnTable  fn_table;
            RUZHOUXIE(no_unique_address) ArgTable arg_table;
        };
    }

    template<class FnTable, class ArgTable>
    struct invoke_view : detail::invoke_view_storage<FnTable, ArgTable>, view_interface<invoke_view<FnTable, ArgTable>>
    {
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
    };

    template<class FnTable, class ArgTable>
    invoke_view(FnTable, ArgTable) -> invoke_view<FnTable, ArgTable>;

    namespace detail
    {
        template<class FnTable>
        struct invoke_t : adaptor_closure<invoke_t<FnTable>>
        {
            FnTable fn_table;

            template<class ArgTable>
            constexpr decltype(auto) operator()(ArgTable&& arg_table)const
            {
                return invoke_view{ fn_table, arg_table };
            }
        };

        // template<class Fn>
        // struct transform_t : adaptor_closure<transform_t<Fn>>
        // {
        //     Fn fn;

        //     template<class V>
        //     constexpr decltype(auto) operator()(V&& vec)const
        //     {

        //         return invoke_view{ array{}, arg_table };
        //     }
        // };
    }

    template<class FnTable>
    constexpr auto invoke(FnTable&& fn_table)
    {
        return detail::invoke_t<FnTable>{ FWD(fn_table) };
    }


}

#include "macro_undef.hpp"
#endif