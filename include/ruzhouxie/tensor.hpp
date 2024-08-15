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

    template<class M, class V>
    constexpr decltype(auto) mat_mul_vec(M&& mat, V&& vec)
    {
        constexpr size_t n = child_count<M>;
        return zip_transform([](auto&& row, auto&& vec){ return dot(FWD(row), FWD(vec)); }, FWD(mat), FWD(vec) | repeat<n>);
    }

    constexpr auto vec_mul_mat(auto&& _vec, auto&& _mat)
	{
	    return mat_mul_vec(FWD(_mat) | transpose<>, FWD(_vec));
	};

    template<class L, class R>
    constexpr auto mat_mul(L&& l, R&& r)
	{
        constexpr size_t n = child_count<L>;
		return  zip_transform([](auto&& row, auto&& mat){ return vec_mul_mat(FWD(row), FWD(mat)); }, FWD(l), FWD(r) | repeat<n>);
	};
}

#include "macro_undef.hpp"
#endif