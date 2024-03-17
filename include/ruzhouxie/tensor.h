#ifndef RUZHOUXIE_TENSOR_H
#define RUZHOUXIE_TENSOR_H

#include "transform.h"

#include "macro_define.h"

namespace ruzhouxie
{
    using defalut_value_t = double;

    template<size_t Dim, typename T = defalut_value_t>
    using vec = view<array<T, Dim>>;

    template<size_t NRow, size_t NColumn = NRow, typename T = defalut_value_t>
    using rmat = view<array<array<T, NColumn>, NRow>>;

    template<size_t NRow, size_t NColumn = NRow, typename T = defalut_value_t>
    using cmat = view<relayout_view<
	    array<array<T, NRow>, NColumn>,
	    transpose<0uz, 1uz>.relayout(default_layout<array<array<T, NRow>, NColumn>>)
		>>;

    template<size_t NRow, size_t NColumn = NRow, typename T = defalut_value_t>
    using mat = rmat<NRow, NColumn>;

    template<typename T>
    using quat = view<array<T, 4>>;
}

namespace ruzhouxie
{
    template<typename T>
    RUZHOUXIE_INLINE constexpr auto sqrt(T x)noexcept
	{
	    return std::sqrt(x);
	};

    RUZHOUXIE_INLINE constexpr decltype(auto) tree_invoke(auto&& fn, auto&&...args)noexcept
	{
	    if constexpr (requires{ fn(FWD(args)...); })
		{
		    return fn(FWD(args)...);;
		}
	    else
		{
		    return zip_transform([=](auto&&...args) { return tree_invoke(fn, FWD(args)...); }, FWD(args)...);
		}
	}

    RUZHOUXIE_INLINE constexpr decltype(auto) add(auto&& l, auto&& r)noexcept
	{
	    return tree_invoke(std::plus<>{}, FWD(l), FWD(r));
	}

    RUZHOUXIE_INLINE constexpr decltype(auto) sub(auto&& l, auto&& r)noexcept
	{
	    return tree_invoke(std::minus<>{}, FWD(l), FWD(r));
	}

    RUZHOUXIE_INLINE constexpr decltype(auto) mul(auto&& l, auto&& r)noexcept
	{
	    return tree_invoke(std::multiplies<>{}, FWD(l), FWD(r));
	}

    RUZHOUXIE_INLINE constexpr decltype(auto) div(auto&& l, auto&& r)noexcept
	{
	    return tree_invoke(std::divides<>{}, FWD(l), FWD(r));
	}


    RUZHOUXIE_INLINE constexpr decltype(auto) dot(auto&& l, auto&& r)noexcept
	{
	    auto mul = zip_transform(std::multiplies<>{}, FWD(l), FWD(r));
	    return [&]<size_t...I>(std::index_sequence<I...>)
		{
		    return (... + (mul | child<I>));
		}(std::make_index_sequence<child_count<decltype(mul)>>{});
	}

    RUZHOUXIE_INLINE constexpr decltype(auto) len_sq(auto&& vector)noexcept
	{
	    return dot(FWD(vector), FWD(vector));
	}

    template<typename Vec>
    RUZHOUXIE_INLINE constexpr decltype(auto) len(Vec&& vector)noexcept
	{
	    if constexpr(child_count<Vec> == 1uz)
		{
		    return FWD(vector) | child<0>;
		}
	    else
		{
		    return sqrt(len_sq(vector));
		}
	}

    RUZHOUXIE_INLINE constexpr decltype(auto) mat_mul_vec(auto&& _mat, auto&& vec)
	{
	    return FWD(_mat) | transform([&](auto&& row) { return dot(row, vec); });
	}

    RUZHOUXIE_INLINE constexpr decltype(auto) vec_mul_mat(auto&& _vec, auto&& _mat)
	{
	    return mat_mul_vec(FWD(_mat) | transpose<>, FWD(_vec));
	}

    RUZHOUXIE_INLINE constexpr auto mat_mul(auto&& l, auto&& r)
	{
	    return FWD(l) | transform([&](auto&& l_row) { return vec_mul_mat(FWD(l_row), r); });
	}

    RUZHOUXIE_INLINE constexpr auto quat_to_mat()
	{
		
	}
}

namespace ruzhouxie
{
    template<typename T>
    concept tree_view = std::derived_from<purified<T>, view_interface<purified<T>>>;

    template<typename L, typename R> requires tree_view<L> || tree_view<R>
    RUZHOUXIE_INLINE constexpr decltype(auto) operator+(L&& l, R&& r)noexcept
	{
	    return add(FWD(l), FWD(r));
	}

    template<typename L, typename R> requires tree_view<L> || tree_view<R>
    RUZHOUXIE_INLINE constexpr decltype(auto) operator-(L&& l, R&& r)noexcept
	{
	    return sub(FWD(l), FWD(r));
	}

    template<typename L, typename R> requires tree_view<L> || tree_view<R>
    RUZHOUXIE_INLINE constexpr decltype(auto) operator*(L&& l, R&& r)noexcept
	{
	    if constexpr (terminal<L>)
		{
		    return tree_invoke([&](auto&& e) { return mul(l, FWD(e)); }, FWD(r));
		}
	    else if constexpr (terminal<R>)
		{
		    return tree_invoke([&](auto&& e) { return mul(FWD(e), r); }, FWD(l));
		}
	    else 
		{
		    return mul(FWD(l), FWD(r));
		}
	}

    template<typename L, typename R> requires tree_view<L> || tree_view<R>
    RUZHOUXIE_INLINE constexpr decltype(auto) operator/(L&& l, R&& r)noexcept
	{
	    if constexpr (terminal<R>)
		{
		    return tree_invoke([&](auto&& e) { return FWD(e) * r; }, FWD(l));
		}
	    else
		{
		    return mul(FWD(l), FWD(r));
		}
	}
}

#include "macro_undef.h"
#endif