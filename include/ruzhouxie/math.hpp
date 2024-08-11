#ifndef RUZHOUXIE_MATH_HPP
#define RUZHOUXIE_MATH_HPP

#include <cmath>

#include "general.hpp"

#include "macro_define.hpp"

namespace rzx
{
    constexpr size_t normalize_index(std::integral auto index, size_t size)noexcept
    {
        if(index >= 0)
        {
            return static_cast<size_t>(index % size);
        }
        else
        {
            return static_cast<size_t>(size - -index % size);
        }
    }

    constexpr auto&& max(auto&& arg0, auto&&...rest)
    {
        if constexpr(sizeof...(rest) == 0)
        {
            return FWD(arg0);
        }
        else
        {
            auto&& rest_max = max(FWD(rest)...);
            if(arg0 > rest_max)
            {
                return FWD(arg0);
            }
            else
            {
                return FWD(rest_max);
            }
        }
    }

    constexpr auto&& min(auto&& arg0, auto&&...rest)
    {
        if constexpr(sizeof...(rest) == 0)
        {
            return FWD(arg0);
        }
        else
        {
            auto&& rest_max = max(FWD(rest)...);
            if(arg0 < rest_max)
            {
                return FWD(arg0);
            }
            else
            {
                return FWD(rest_max);
            }
        }
    }

    template<typename T>
    constexpr auto sqrt(T x)noexcept
	{
	    return std::sqrt(x);
	};
}

#include "macro_undef.hpp"
#endif