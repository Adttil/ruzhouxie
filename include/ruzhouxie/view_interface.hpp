#ifndef RUZHOUXIE_VIEW_INTERFACE_HPP
#define RUZHOUXIE_VIEW_INTERFACE_HPP

#include "general.hpp"
#include "simplify.hpp"

#include "macro_define.hpp"

namespace rzx 
{
    template<class View>
    struct view_interface
    {};

    template<class T>
    concept viewed = requires(std::remove_cvref_t<T>& t)
    {
        { []<class V>(view_interface<V>&)->V*{}(t) } -> std::same_as<std::remove_cvref_t<T>*>;
    };

    namespace detail
    {
        template<class T>
        struct view_storage
        {
            T base;
        };
    }

    template<class T>
    struct view : detail::view_storage<T>, view_interface<view<T>>
    {
        template<size_t I, class Self>
        constexpr decltype(auto) get(this Self&& self)
        {
            if constexpr(I < child_count<T>)
            {
                return FWD(self, base) | child<I>;
            }
            else
            {
                return end();
            } 
        }

        template<auto Usage, auto Layout, class Self>
        constexpr decltype(auto) simplify(this Self&& self)
        {
            return FWD(self, base) | rzx::simplify<Usage, Layout>;
        }
    };

    template<class T>
    view(T) -> view<T>;

    template<class T>
    concept wrapped = requires(std::remove_cvref_t<T>& t)
    {
        { []<class V>(view<V>&)->view<V>*{}(t) } -> std::same_as<std::remove_cvref_t<T>*>;
    };

    namespace detail 
    {
        struct refer_t : adaptor_closure<refer_t>
        {
            template<class T>
            constexpr decltype(auto) operator()(T&& t) const
            {
                if constexpr(std::is_lvalue_reference_v<T>)
                {
                    return t;
                }
                else
                {
                    return view<T&&>{ FWD(t) };
                }
            }
        };
    }

    inline constexpr detail::refer_t refer{};
}

#include "macro_undef.hpp"
#endif