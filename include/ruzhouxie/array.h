#ifndef RUZHOUXIE_ARRAY_H
#define RUZHOUXIE_ARRAY_H

#include "general.h"
#include <concepts>
#include <tuple>

#include "macro_define.h"

#if __STDC_HOSTED__

#include <array>

namespace ruzhouxie
{
	using std::array;
}

#else// For freestanding implementation which maybe do not has std::array.

namespace ruzhouxie
{
	// This simple array is only for this library. 
	// It is not guaranteed to have exactly the same behavior as std::array.
	template<typename T, size_t N>
	struct array
	{
		using value_type = T;

		T data[N];

		template<typename Self>
		constexpr auto&& operator[](this Self&& self, size_t i)noexcept
		{
			return fwd<Self&&, T>(self.data[i]);
		}

		static constexpr size_t size()noexcept
		{
			return N;
		}

		constexpr auto begin(this auto&& self)noexcept
		{
			return self.data;
		}

		constexpr auto end(this auto&& self)noexcept
		{
			return self.data + N;
		}

		template<size_t i, specified<array> Self>
		friend constexpr decltype(auto) get(Self&& self)noexcept
		{
			return FWD(self)[i];
		}

		friend constexpr bool operator==(const array&, const array&) = default;
	};

	template<typename T>
	struct array<T, 0uz>
	{
		using value_type = T;

		static constexpr size_t size()
		{
			return 0uz;
		}

		constexpr T& operator[](size_t i) noexcept;
		constexpr const T& operator[](size_t i)const noexcept;

		template<typename Self>
		RUZHOUXIE_INLINE constexpr T* begin()noexcept
		{
			return nullptr;
		}

		template<typename Self>
		RUZHOUXIE_INLINE constexpr const T* begin()const noexcept
		{
			return nullptr;
		}

		template<typename Self>
		RUZHOUXIE_INLINE constexpr T* end()noexcept
		{
			return nullptr;
		}

		template<typename Self>
		RUZHOUXIE_INLINE constexpr const T* end()const noexcept
		{
			return nullptr;
		}

		friend constexpr bool operator==(const array&, const array&) = default;
	};

	template<typename...T>
	array(T...) -> array<std::common_type_t<std::decay_t<T>...>, sizeof...(T)>;
}

template<typename T, size_t N>
struct std::tuple_size<ruzhouxie::array<T, N>>{
	static constexpr size_t value = N;
};

template<size_t I, typename T, size_t N>
struct std::tuple_element<I, ruzhouxie::array<T, N>>{
	using type = T;
};

#endif

namespace ruzhouxie::detail
{
	template<size_t N, typename A>
	consteval auto array_take(const A& arr)
	{
		using type = A::value_type;
		array<type, N> result{};
		for (size_t i = 0; i < N; ++i)
		{
			result[i] = arr[i];
		}
		return result;
	}

	template<size_t N, typename A>
	consteval auto array_drop(const A& arr)
	{
		using type = A::value_type;
		array<type, std::tuple_size_v<A> - N> result{};
		for(size_t i = 0; i < std::tuple_size_v<A> - N; ++i)
		{
			result[i] = arr[i + N];
		}
		return result;
	}

	template<typename A1, typename A2>
	consteval auto concat_2_array(const A1& arr1, const A2& arr2)
	{
		using type1 = A1::value_type;
		using type2 = A2::value_type;
		constexpr size_t n1 = std::tuple_size_v<A1>;
		constexpr size_t n2 = std::tuple_size_v<A2>;
		array<std::common_type_t<type1, type2>, n1 + n2> result{};

		for (size_t i = 0; i < n1; ++i)
		{
			result[i] = arr1[i];
		}
		for (size_t i = 0; i < n2; ++i)
		{
			result[n1 + i] = arr2[i];
		}

		return result;
	}

	template<typename A, typename...Rest>
	consteval auto concat_array(const A& arr, const Rest&...rest)
	{
		if constexpr (sizeof...(rest) == 0)
		{
			return arr;
		}
		else
		{
			return concat_2_array(arr, concat_array(rest...));
		}
	}

	// template<typename T, size_t N1, size_t N2>
	// consteval auto merge_array_size(const array<T, N1>& arr1, const array<T, N2>& arr2)
	// {
	// 	size_t n = 0;
	// 	auto i1 = arr1.begin();
	// 	auto i2 = arr2.begin();
	// 	while (i1 != arr1.end() && i2 != arr2.end())
	// 	{
	// 		++n;
	// 		if (*i1 < *i2)
	// 		{
	// 			++i1;
	// 		}
	// 		else if (*i1 > *i2)
	// 		{
	// 			++i2;
	// 		}
	// 		else
	// 		{
	// 			++i1;
	// 			++i2;
	// 		}
	// 	}
	// 	n += arr1.end() - i1 + (arr2.end() - i2);
	// 	return n;
	// }
	//
	// template<array arr1, array arr2>
	// consteval auto merge_2_array()
	// {
	// 	using value_type = purified<decltype(arr1)>::value_type;
	// 	constexpr size_t n = merge_array_size(arr1, arr2);
	// 	array<value_type, n> result{};
	//
	// 	auto i = result.begin();
	// 	auto i1 = arr1.begin();
	// 	auto i2 = arr2.begin();
	// 	while (i1 != arr1.end() && i2 != arr2.end())
	// 	{
	// 		if (*i1 < *i2)
	// 		{
	// 			*i++ = *i1++;
	// 		}
	// 		else if (*i1 > *i2)
	// 		{
	// 			*i++ = *i2++;
	// 		}
	// 		else
	// 		{
	// 			*i++ = (i1++, *i2++);
	// 		}
	// 	}
	//
	// 	while (i1 != arr1.end())
	// 	{
	// 		*i++ = *i1++;
	// 	}
	// 	while (i2 != arr2.end())
	// 	{
	// 		*i++ = *i2++;
	// 	}
	// 	return result;
	// }
	//
	// template<array...Arr>
	// consteval auto merge_array()
	// {
	// 	if constexpr (sizeof...(Arr) == 0)
	// 	{
	// 		return array<size_t, 0>{};
	// 	}
	// 	else if constexpr (sizeof...(Arr) == 1)
	// 	{
	// 		return (..., Arr);
	// 	}
	// 	else return[]<size_t...I>(std::index_sequence<I...>)
	// 	{
	// 		return merge_array<merge_2_array<arg_at<0>(Arr...), arg_at<1>(Arr...)>(), arg_at<I + 2>(Arr...)...>();
	// 	}(std::make_index_sequence<sizeof...(Arr) - 2>{});
	// }	
}

#include "macro_undef.h"
#endif