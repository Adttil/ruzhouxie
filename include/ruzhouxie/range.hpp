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
        
    };
}

#include "macro_undef.hpp"
#endif