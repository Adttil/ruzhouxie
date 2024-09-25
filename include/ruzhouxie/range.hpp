#ifndef RUZHOUXIE_RANGE_HPP
#define RUZHOUXIE_RANGE_HPP

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
    inline constexpr auto next(const auto& value)
    {
        auto copy = value;
        return ++copy;
    }

    inline constexpr auto prev(const auto& value)
    {
        auto copy = value;
        return --copy;
    }

    template<auto Start, auto End>
    struct range_view : view_interface<range_view<Start, End>>
    {
        template<size_t I>
        constexpr auto get()const
        {
            if constexpr(I == 0uz)
            {
                return constant_t<Start>{};
            }
            else
            {
                constexpr auto result = next(range_view{}.template get<I - 1uz>().value);
                if constexpr(result < End)
                {
                    return constant_t<result>{};
                }
                else
                {
                    return end();
                }
            }
        }
    };

    template<auto Start, auto End = 0uz>
    inline constexpr range_view<min(Start, End), max(Start, End)> range{};

    struct enumerate_t : adaptor_closure<enumerate_t>
    {
        template<class T>
        constexpr auto operator()(T&& t)const
        {
            return zip(range<child_count<T>>, FWD(t)); 
        }
    };

    inline constexpr enumerate_t enumerate{};
}

#include "macro_undef.hpp"
#endif