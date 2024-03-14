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
}

#include "macro_undef.h"
#endif