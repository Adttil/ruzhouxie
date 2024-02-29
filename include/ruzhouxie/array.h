#ifndef RUZHOUXIE_ARRAY_H
#define RUZHOUXIE_ARRAY_H

#include "general.h"
#include <concepts>
#include <tuple>

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
		constexpr decltype(auto) operator[](this Self&& self, size_t i)noexcept
		{
			return FWD(self, data[i]);
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
			return FWD(self, data)[i];
		}

		template<std::same_as<T>...Args>
		constexpr bool coutain(const Args&...args)const noexcept
		{
			for (const auto& e : *this)
			{
				if ((false || ... || (e == args)))
				{
					return true;
				}
			}
			return false;
		}

		friend constexpr bool operator==(const array&, const array&) = default;

		template<size_t M> requires(M <= N)
			constexpr array<T, M> take() const
		{
			array<T, M> result{};
			for (size_t i = 0; i < M; ++i)
			{
				result[i] = (*this)[i];
			}
			return result;
		}
	};

	template<typename T>
	struct array<T, 0uz>
	{
		using value_type = T;

		static constexpr size_t size()
		{
			return 0uz;
		}

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

		template<std::same_as<T>...Args>
		RUZHOUXIE_INLINE constexpr bool coutain(const Args&...args)const noexcept
		{
			return false;
		}

		friend constexpr bool operator==(const array&, const array&) = default;

		template<size_t M> requires(M <= 0uz)
			constexpr array take() const
		{
			return array{};
		}
	};

	template<typename...T>
	array(T...) -> array<std::common_type_t<std::decay_t<T>...>, sizeof...(T)>;
}
#endif

namespace ruzhouxie
{
	template<size_t Count, typename T, size_t N>
	constexpr auto array_take(const array<T, N>& arr) noexcept
	{
		array<T, Count> result{};
		for (size_t i = 0; i < Count; ++i)
		{
			result[i] = arr[i];
		}
		return result;
	}

	template<typename T, size_t N1, size_t N2>
	constexpr auto merge_array_size(const array<T, N1>& arr1, const array<T, N2>& arr2)noexcept
	{
		size_t n = 0;
		auto i1 = arr1.begin();
		auto i2 = arr2.begin();
		while (i1 != arr1.end() && i2 != arr2.end())
		{
			++n;
			if (*i1 < *i2)
			{
				++i1;
			}
			else if (*i1 > *i2)
			{
				++i2;
			}
			else
			{
				++i1;
				++i2;
			}
		}
		n += arr1.end() - i1 + (arr2.end() - i2);
		return n;
	}

	template<array arr1, array arr2>
	constexpr auto merge_2_array()
	{
		using value_type = purified<decltype(arr1)>::value_type;
		constexpr size_t n = merge_array_size(arr1, arr2);
		array<value_type, n> result{};

		auto i = result.begin();
		auto i1 = arr1.begin();
		auto i2 = arr2.begin();
		while (i1 != arr1.end() && i2 != arr2.end())
		{
			if (*i1 < *i2)
			{
				*i++ = *i1++;
			}
			else if (*i1 > *i2)
			{
				*i++ = *i2++;
			}
			else
			{
				*i++ = (i1++, *i2++);
			}
		}

		while (i1 != arr1.end())
		{
			*i++ = *i1++;
		}
		while (i2 != arr2.end())
		{
			*i++ = *i2++;
		}
		return result;
	}

	template<array...Arr>
	constexpr auto merge_array()
	{
		if constexpr (sizeof...(Arr) == 0)
		{
			return array<size_t, 0>{};
		}
		else if constexpr (sizeof...(Arr) == 1)
		{
			return (..., Arr);
		}
		else return[]<size_t...I>(std::index_sequence<I...>)
		{
			return merge_array<merge_2_array<arg_at<0>(Arr...), arg_at<1>(Arr...)>(), arg_at<I + 2>(Arr...)...>();
		}(std::make_index_sequence<sizeof...(Arr) - 2>{});
	}

	template<typename T, size_t N1, size_t N2>
	constexpr auto concat_2_array(const array<T, N1>& arr1, const array<T, N2>& arr2)
	{
		array<T, N1 + N2> result{};

		for (size_t i = 0; i < N1; ++i)
		{
			result[i] = arr1[i];
		}
		for (size_t i = 0; i < N2; ++i)
		{
			result[N1 + i] = arr2[i];
		}

		return result;
	}

	template<typename T, size_t N, size_t...Rest>
	constexpr auto concat_array(const array<T, N>& arr, const array<T, Rest>&...rest)
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

	template<size_t Len, typename T, size_t N>
	constexpr auto array_drop(const array<T, N>& arr)noexcept
	{
		std::array<T, N - Len> result{};
		for(size_t i = 0; i < N - Len; ++i)
		{
			result[i] = arr[i + Len];
		}
		return result;
	}

	template<typename T>
	concept indices = std::same_as<purified<T>, array<typename  purified<T>::value_type, std::tuple_size_v<purified<T>>>>
		&& std::integral<typename  purified<T>::value_type>;
}

#endif