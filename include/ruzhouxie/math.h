#ifndef RUZHOUXIE_MATH_H
#define RUZHOUXIE_MATH_H

#include "general.h"

#include "macro_define.h"

namespace ruzhouxie
{
    RUZHOUXIE_INLINE constexpr size_t normalize_index(std::integral auto index, size_t size)noexcept
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

    RUZHOUXIE_INLINE constexpr auto&& max(auto&& arg0, auto&&...rest)
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

    RUZHOUXIE_INLINE constexpr auto&& min(auto&& arg0, auto&&...rest)
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
    RUZHOUXIE_INLINE constexpr auto sqrt(T x)noexcept
	{
	    return std::sqrt(x);
	};
}

#include "macro_undef.h"
#endif