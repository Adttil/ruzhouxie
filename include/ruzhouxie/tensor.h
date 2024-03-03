#ifndef RUZHOUXIE_TENSOR_H
#define RUZHOUXIE_TENSOR_H

#include "transform.h"

#include "macro_define.h"

namespace ruzhouxie
{
	namespace detail
	{
		template<size_t M, size_t N>
		constexpr auto tensor_layout_add_prefix(const array<size_t, M>& layout, const array<size_t, N>& prefix)
		{
			return concat_array(prefix, layout);
		}

		template<typename T, size_t N> requires (tensor_rank<T> >= 2uz)
			constexpr auto tensor_layout_add_prefix(const T& layout, const array<size_t, N>& prefix)
		{
			return[&]<size_t...I>(std::index_sequence<I...>)
			{
				return array{ tensor_layout_add_prefix(layout | child<I>, prefix)... };
			}(std::make_index_sequence<child_count<T>>{});
		}
	}

	template<typename T>
	constexpr auto default_tensor_layout
	{
		[]()
		{
			if constexpr (terminal<T>)
			{
				return array<size_t, 0uz>{};
			}
			else return[]<size_t...I>(std::index_sequence<I...>)
			{
				return array{ detail::tensor_layout_add_prefix(default_tensor_layout<child_type<T, I>>, array{I})... };
			}(std::make_index_sequence<child_count<T>>{});
		}()
	};
}

namespace ruzhouxie
{
	template<size_t I, size_t Axis = 0uz, typename T = void>
	constexpr auto component_copy(const T& t)
	{
		if constexpr (Axis == 0uz)
		{
			static_assert(I < child_count<T>, "Component index out of range.");
			return t | child<I>;
		}
		else
		{
			static_assert(branched<T>, "Axis index out of range.");
			return[&]<size_t...J>(std::index_sequence<J...>)
			{
				return tuple{ component_copy<I, Axis - 1uz>(t | child<J>)... };
			}(std::make_index_sequence<child_count<T>>{});
		}
	}

	template<size_t Axis1 = 0uz, size_t Axis2 = Axis1 + 1uz, typename T = void>
	constexpr auto transpose_copy(const T& t)
	{
		if constexpr (Axis1 == 0uz)
		{
			constexpr size_t N = tensor_shape<T>[Axis2];
			return[&]<size_t...I>(std::index_sequence<I...>)
			{
				return tuple{ component_copy<I, Axis2>(t)... };
			}(std::make_index_sequence<N>{});
		}
		else return[&]<size_t...I>(std::index_sequence<I...>)
		{
			return tuple{ transpose_copy<Axis1 - 1uz, Axis2 - 1uz>(t | child<I>)... };
		}(std::make_index_sequence<child_count<T>>{});
	}
}

//component
namespace ruzhouxie
{
	namespace detail
	{
		template<size_t I, size_t Axis>
		struct component_t
		{
			template<typename T>
			RUZHOUXIE_INLINE constexpr decltype(auto) operator()(T&& t) const
			{
				if constexpr (Axis == 0)
				{
					return t | child<I>;
				}
				else
				{
					constexpr auto tensor_layout = default_tensor_layout<T>;
					return relayout_view<T, component_copy<I, Axis>(tensor_layout)>
					{
						{}, FWD(t)
					};
				}
			}
		};
	};

	inline namespace functors
	{
		template<size_t J, size_t Axis>
		inline constexpr pipe_closure<detail::component_t<J, Axis>> component{};
	}
}

//transpose
namespace ruzhouxie
{
	namespace detail
	{
		template<size_t Axis1 = 0uz, size_t Axis2 = Axis1 + 1uz>
		struct transpose_t
		{
			template<typename T>
			RUZHOUXIE_INLINE constexpr decltype(auto) operator()(T&& t) const
			{
				constexpr auto tensor_layout = default_tensor_layout<T>;
				return relayout_view<T, transpose_copy<Axis1, Axis2>(tensor_layout)>
				{
					{}, FWD(t)
				};
			}
		};
	};

	template<size_t Axis1 = 0uz, size_t Axis2 = Axis1 + 1uz>
	inline constexpr pipe_closure<detail::transpose_t<Axis1, Axis2>> transpose{};
}

namespace ruzhouxie
{
	using defalut_value_t = double;

	template<size_t Dim, typename T = defalut_value_t>
	using vec = view<array<T, Dim>>;

	template<size_t NRow, size_t NColumn = NRow, typename T = defalut_value_t>
	using rmat = view<array<array<T, NColumn>, NRow>>;

	template<size_t NRow, size_t NColumn = NRow, typename T = defalut_value_t>
	using cmat = view<detail::relayout_view<
		array<array<T, NRow>, NColumn>,
		transpose_copy<0uz, 1uz>(default_tensor_layout<array<array<T, NRow>, NColumn>>)
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
		return tree_invoke(std::plus<>{}, l, r);
	}

	RUZHOUXIE_INLINE constexpr decltype(auto) sub(auto&& l, auto&& r)noexcept
	{
		return tree_invoke(std::minus<>{}, l, r);
	}

	RUZHOUXIE_INLINE constexpr decltype(auto) mul(auto&& l, auto&& r)noexcept
	{
		return tree_invoke(std::multiplies<>{}, l, r);
	}

	RUZHOUXIE_INLINE constexpr decltype(auto) div(auto&& l, auto&& r)noexcept
	{
		return tree_invoke(std::divides<>{}, l, r);
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
			return FWD(vector) | child<0, empty_id_set>;
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
	concept tree_view = std::derived_from<purified<T>, detail::view_base<purified<T>>>;

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