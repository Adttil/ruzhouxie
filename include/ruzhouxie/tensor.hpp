#ifndef RUZHOUXIE_TENSOR_HPP
#define RUZHOUXIE_TENSOR_HPP

#include "general.hpp"
#include "invoke.hpp"
#include "make.hpp"

#include "macro_define.hpp"

namespace rzx 
{
    template<class L, class R>
    constexpr decltype(auto) dot(L&& left, R&& right)
    {
        return [&]<size_t...I>(std::index_sequence<I...>) -> decltype(auto)
        {
            return (0 + ... + ((left | child<I>) * (right | child<I>)));
        }(std::make_index_sequence<rzx::min(child_count<L>, child_count<R>)>{});
    }

    template<class V, class M>
    constexpr decltype(auto) vec_mul_mat(V&& vec, M&& mat)
    {
        
    }
}

#include "macro_undef.hpp"
#endif