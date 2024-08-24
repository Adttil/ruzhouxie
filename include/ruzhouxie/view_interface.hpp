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
        // template<class Self>
        // constexpr decltype(auto) self(this Self&& self)
        // {
        //     if constexpr(std::is_object_v<Self> && std::is_reference_v<T>)
        //     {
        //         return view{ FWD(self) };
        //     }
        //     else
        //     {
        //         return FWD(self);
        //     }
        // }

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

        template<auto UsageTable, typename Self>
        constexpr auto simplifier(this Self&& self)
        {
            return FWD(self, base) | get_simplifier<UsageTable>;
        }

        // template<auto UsageTable, typename Self>
        // constexpr decltype(auto) simplified_data(this Self&& self)
        // {
        //     return FWD(self, base) | rzx::simplified_data<UsageTable>;
        // }

        // template<auto UsageTable, derived_from<view> Self>
        // friend constexpr auto get_simplified_layout(type_tag<Self>)
        // {
        //     return simplified_layout<T, UsageTable>;
        // }
    };

    template<class T>
    view(T) -> view<T>;

    template<class T>
    concept wrapped = requires(std::remove_cvref_t<T>& t)
    {
        { []<class V>(view<V>&)->view<V>*{}(t) } -> std::same_as<std::remove_cvref_t<T>*>;
    };

    template<class T>
    using unwrap_t = decltype([]{
        if constexpr(not wrapped<T>)
        {
            return type_tag<T>{};
        }
        else if constexpr(std::is_lvalue_reference_v<T&&> && std::is_object_v<decltype(T::base)>)
        {
            return type_tag<decltype(T::base)>{};
        }
        else
        {
            return type_tag<decltype(FWD(std::declval<T>(), base))>{};
        }
    }())::type;

    template<class T>
    constexpr decltype(auto) unwrap(T&& t)
    {
        if constexpr(not wrapped<T>)
        {
            return FWD(t);
        }
        else
        {
            return FWD(t, base);
        }
    }

    namespace detail 
    {
        struct wrap_t : adaptor_closure<wrap_t>
        {
            template<class T>
            constexpr view<unwrap_t<T>> operator()(T&& t) const
            {
                return { rzx::unwrap(FWD(t)) };
            }
        };

        struct refer_t : adaptor_closure<refer_t>
        {
            template<class T>
            constexpr view<unwrap_t<T>&&> operator()(T&& t) const
            {
                return { rzx::unwrap(FWD(t)) };
            }
        };
    }

    inline constexpr detail::wrap_t wrap{};

    inline constexpr detail::refer_t refer{};
}

#include "macro_undef.hpp"
#endif