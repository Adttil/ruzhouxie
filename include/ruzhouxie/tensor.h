#ifndef RUZHOUXIE_TENSOR_H
#define RUZHOUXIE_TENSOR_H

#include "general.h"
#include "math.h"
#include "relayout.h"
//#include "transform.h"
#include "invoke3.h"

#include "macro_define.h"

namespace ruzhouxie::detail
{
	template<size_t NRow, size_t NColumn>
	RUZHOUXIE_CONSTEVAL auto rmat_layout()
	{
		return []<size_t...I>(std::index_sequence<I...>)
		{
			auto row_layout = []<size_t I_, size_t...J>(std::index_sequence<J...>)
			{
				return tuple{ array{ I_ * NColumn + J }... };
			};

			return make_tuple(row_layout.template operator()<I>(std::make_index_sequence<NColumn>{})...);
		}(std::make_index_sequence<NRow>{});
	}

	template<size_t NRow, size_t NColumn>
	RUZHOUXIE_CONSTEVAL auto cmat_layout()
	{
		return []<size_t...I>(std::index_sequence<I...>)
		{
			auto row_layout = []<size_t I_, size_t...J>(std::index_sequence<J...>)
			{
				return tuple{ array{ I_ + J * NRow }... };
			};

			return make_tuple(row_layout.template operator()<I>(std::make_index_sequence<NColumn>{})...);
		}(std::make_index_sequence<NRow>{});
	}
}

namespace ruzhouxie
{
    using defalut_value_t = double;

    template<size_t Dim, typename T = defalut_value_t>
    using vec = view<array<T, Dim>>;

    template<size_t NRow, size_t NColumn = NRow, typename T = defalut_value_t>
    using rmat = relayout_view<array<T, NColumn * NRow>, detail::rmat_layout<NRow, NColumn>()>;

    template<size_t NRow, size_t NColumn = NRow, typename T = defalut_value_t>
    using cmat = relayout_view<array<T, NColumn * NRow>, detail::cmat_layout<NRow, NColumn>()>;

    template<size_t NRow, size_t NColumn = NRow, typename T = defalut_value_t>
    using mat = rmat<NRow, NColumn>;

    template<typename T = defalut_value_t>
    using quat = view<array<T, 4>>;
}

namespace ruzhouxie
{
    RUZHOUXIE_INLINE constexpr decltype(auto) tree_invoke(auto&& fn, auto&&...args)noexcept
	{
		//if constexpr (requires{ fn(FWD(args)...); })
	    if constexpr ((... || terminal<decltype(args)>))
		{
		    return fn(FWD(args)...);
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


    inline constexpr auto dot = [](auto&& l, auto&& r)
	{
	    auto mul = zip_transform(std::multiplies<>{}, FWD(l), FWD(r));
	    return [&]<size_t...I>(std::index_sequence<I...>)
		{
		    return (... + (mul | child<I>));
		}(std::make_index_sequence<child_count<decltype(mul)>>{});
	};

	inline constexpr auto cross = []<typename L, typename R>(L&& l, R&& r)
	{
		if constexpr(child_count<L> == 2uz && child_count<R> == 2uz)
		{
			return child<0uz>(l) * child<1uz>(r) - child<1uz>(l) * child<0uz>(r);
		}
		else if constexpr(child_count<L> == 3uz && child_count<R> == 3uz)
		{
			constexpr auto layout = tuple
			{
				tuple{ array{ 0uz, 1uz }, array{ 1uz, 2uz }, array{ 1uz, 1uz }, array{ 0uz, 2uz } },
				tuple{ array{ 0uz, 2uz }, array{ 1uz, 0uz }, array{ 1uz, 2uz }, array{ 0uz, 0uz } },
				tuple{ array{ 0uz, 0uz }, array{ 1uz, 1uz }, array{ 1uz, 0uz }, array{ 0uz, 1uz } }
			};
			return tuple<L, R>{ FWD(l), FWD(r) } | relayout<layout> | transform([](auto&& args)
			{
				return child<0uz>(args) * child<1uz>(args) - child<2uz>(args) * child<3uz>(args);
			});
		}
		else
		{
			static_assert(child_count<L> == 2uz && child_count<R> == 2uz);
		}
	};

    inline constexpr auto len_sq = [](auto&& vector)
	{
	    return dot(FWD(vector), FWD(vector));
	};

    inline constexpr auto len = []<typename Vec>(Vec&& vector)
	{
	    if constexpr(child_count<Vec> == 1uz)
		{
		    return FWD(vector) | child<0>;
		}
	    else
		{
		    return sqrt(len_sq(vector));
		}
	};

    inline constexpr auto mat_mul_vec = [](auto&& mat, auto&& vec)
	{
		//Compilation is too slow
	    //return zip_transform(dot, FWD(mat), FWD(vec) | repeat<child_count<M>>);
		return FWD(mat) | transform([vec = wrapper{ FWD(vec) }](auto&& row){ return dot(FWD(row), vec.value()); });
	};

    inline constexpr auto vec_mul_mat = [](auto&& _vec, auto&& _mat)
	{
	    return mat_mul_vec(FWD(_mat) | transpose<>, FWD(_vec));
	};

    inline constexpr auto mat_mul = tree_adaptor{[](auto&& l, auto&& r)
	{
		return grouped_cartesian_transform(dot, FWD(l), FWD(r) | transpose<>);
	}};
}

namespace ruzhouxie
{
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