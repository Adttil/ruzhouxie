#ifndef RUZHOUXIE_TREE_BASIC_HPP
#define RUZHOUXIE_TREE_BASIC_HPP

#include "child.hpp"
#include "constant.hpp"
#include "general.hpp"
#include "relayout.hpp"
#include "astrict.hpp"
#include "simplify.hpp"
#include "view_interface.hpp"

#include "macro_define.hpp"

namespace rzx 
{
    namespace detail::make_t_ns
    {
        template<typename T>
        struct make_t;
    }

    template<typename T>
    inline constexpr detail::make_t_ns::make_t<T> make{};
    
    template<typename T>
    struct tuple_maker
    {
        template<typename Arg>
        constexpr T operator()(Arg&& arg)const
        {
            return [&]<size_t...I>(std::index_sequence<I...>)
            {
                auto&& simplified_arg_data = FWD(arg) | simplified_data<>;
                constexpr auto simplified_arg_layout = simplified_layout<Arg>;
                auto simplified_arg = relayout_view<decltype(simplified_arg_data), simplified_arg_layout>{ FWD(simplified_arg_data) };
                //auto&& astrict_arg = FWD(simplify_arg) | astrict<stricture_t::readonly>; 
                return T{ simplified_arg | child<I> | make<std::tuple_element_t<I, T>>... };
            }(std::make_index_sequence<std::tuple_size_v<T>>{});
        }
    };

    namespace detail::make_t_ns
    {        
        template<typename T>
        constexpr auto get_maker(type_tag<T>)noexcept
        {
            return tuple_maker<T>{};
        }
    }

    template<typename T>
    struct detail::make_t_ns::make_t : adaptor_closure<make_t<T>>
    {
        template<typename Arg>
        constexpr T operator()(Arg&& arg)const
        {
            if constexpr(terminal<T>)
            {
                return static_cast<T>(FWD(arg));
            }
            else if constexpr(std::same_as<std::remove_cvref_t<Arg>, T> && requires{ T{ FWD(arg) }; })
            {
                return T{ FWD(arg) };
            }
            else if constexpr(requires{ get_maker(type_tag<T>{}); })
            {
                return get_maker(type_tag<T>{})(FWD(arg));
            }
            else
            {
                static_assert(false, "maker for T not found.");
            }
        }
    };
}

#include "macro_undef.hpp"
#endif