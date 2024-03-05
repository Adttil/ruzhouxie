#ifndef RUZHOUXIE_MATH_H
#define RUZHOUXIE_MATH_H

#include "general.h"

#include "macro_define.h"

namespace ruzhouxie
{
    constexpr size_t modulo(size_t I, size_t N)
    {
        return I % N;
    }
}

#include "macro_undef.h"
#endif