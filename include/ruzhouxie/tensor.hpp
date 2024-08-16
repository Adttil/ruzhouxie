#ifndef RUZHOUXIE_TENSOR_HPP
#define RUZHOUXIE_TENSOR_HPP

#include "general.hpp"
#include "invoke.hpp"
#include "make.hpp"

#include "macro_define.hpp"

namespace rzx 
{
    namespace detail
    {
        struct dot_t : adaptor<dot_t>
        {
            template<class L, class R>
            constexpr decltype(auto) result(L&& left, R&& right)const
            {
                return [&]<size_t...I>(std::index_sequence<I...>) -> decltype(auto)
                {
                    return (... + ((left | child<I>) * (right | child<I>)));
                }(std::make_index_sequence<rzx::min(child_count<L>, child_count<R>)>{});
            }
        };
    }

    inline constexpr detail::dot_t dot{};

    namespace detail
    {
        struct mat_mul_vec_t : adaptor<mat_mul_vec_t>
        {
            template<class M, class V>
            constexpr decltype(auto) result(M&& mat, V&& vec)const
            {
                constexpr size_t row_count = child_count<M>;
                return zip_transform(dot, FWD(mat), FWD(vec) | repeat<row_count>);
            }
        };
    }

    inline constexpr detail::mat_mul_vec_t mat_mul_vec{};

    namespace detail
    {
        struct vec_mul_mat_t : adaptor<vec_mul_mat_t>
        {
            constexpr auto result(auto&& _vec, auto&& _mat)const
	        {
	            return mat_mul_vec(FWD(_mat) | transpose<>, FWD(_vec));
	        };
        };
    }

    inline constexpr detail::vec_mul_mat_t vec_mul_mat{};

    inline constexpr auto mat_mul = []<class L, class R>(L&& l, R&& r)
	{
        constexpr size_t n = child_count<L>;
		return zip_transform(vec_mul_mat, FWD(l), FWD(r) | repeat<n>);
	};
}

#include "macro_undef.hpp"
#endif