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

        struct transform_t : adaptor<transform_t>
        {
            template<class ArgTable, class Fn>
            constexpr decltype(auto) result(ArgTable&& arg_table, Fn&& fn)const
            {
                constexpr size_t n =child_count<ArgTable>;
                return invoke_view<ArgTable, decltype(Fn{} | repeat<n>)>{ FWD(arg_table), Fn{} | repeat<n> };
            }
        };
    }

    inline constexpr detail::invoke_t invoke{};

    inline constexpr detail::transform_t transform{};
}

#include "macro_undef.hpp"
#endif