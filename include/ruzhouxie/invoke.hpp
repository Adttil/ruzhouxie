#ifndef RUZHOUXIE_INVOKE_HPP
#define RUZHOUXIE_INVOKE_HPP

#include "general.hpp"
#include "relayout.hpp"
//#include "simplify.hpp"
#include "make.hpp"
#include "view_interface.hpp"

#include "macro_define.hpp"

namespace rzx 
{
    namespace detail
    {
        template<class Base, class InvokeShape>
        struct invoke_view_storage
        {
            RUZHOUXIE(no_unique_address) Base        base;
            RUZHOUXIE(no_unique_address) InvokeShape invoke_shape;
        };
    }

    template<class Base, class InvokeShape>
    struct invoke_view;
    
    template<class Base, class InvokeShape>
    invoke_view(Base, InvokeShape) -> invoke_view<Base, InvokeShape>;
    
    namespace detail
    {
        struct invoke_t : adaptor<invoke_t>
        {
            template<class V, class InvokeShape>
            constexpr auto result(V&& view, InvokeShape)const
            {
                return invoke_view<unwrap_t<V>, InvokeShape>{ unwrap(FWD(view)) };
            }
        };
    }
    
    inline constexpr detail::invoke_t invoke{};

    template<class Base, class InvokeShape>
    struct invoke_view : detail::invoke_view_storage<Base, InvokeShape>, view_interface<invoke_view<Base, InvokeShape>>
    {
        template<size_t I, class Self>
        constexpr decltype(auto) get(this Self&& self)
        {
            if constexpr(I >= child_count<InvokeShape>)
            {
                return end();
            }
            else if constexpr(terminal<child_type<InvokeShape, I>>)
            {
                return FWD(self, base) | child<I> | rzx::apply([](auto&& fn, auto&&...args){ return FWD(fn)(FWD(args)...); });
            }
            else
            {
                return invoke_view<decltype(FWD(self, base) | child<I>), std::decay_t<child_type<InvokeShape, I>>>
                {
                    FWD(self, fn_table) | child<I>
                };
            }
        }

        template<auto Layout, bool Sequential, class Self>
        constexpr decltype(auto) relayout_seperate(this Self&& self)
        {
            return FWD(self, base) | refer | rzx::relayout_seperate<Layout, Sequential> | invoke(InvokeShape{});
        }
    };

    namespace detail
    {
        struct transform_t : adaptor<transform_t>
        {
            template<class ArgTable, class Fn>
            constexpr decltype(auto) result(ArgTable&& arg_table, Fn&& fn)const
            {
                constexpr size_t n = child_count<ArgTable>;
                return zip(FWD(fn) | repeat<n>, FWD(arg_table)) | invoke(tree_shape<array<size_t, n>>);
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
                return zip(FWD(fn) | repeat<n>, FWD(args), FWD(rest)...) | invoke(tree_shape<array<size_t, n>>);
            }
        };
    }

    inline constexpr detail::zip_transform_t zip_transform{};

    
}

#include "macro_undef.hpp"
#endif