#ifndef RUZHOUXIE_ARRAY_HPP
#define RUZHOUXIE_ARRAY_HPP

#include <cstddef>
#include <tuple>

#include "macro_define.hpp"

#define RUZHOUXIE_DONOT_USE_STD_ARRAY
#if __STDC_HOSTED__ && !defined(RUZHOUXIE_DONOT_USE_STD_ARRAY)

#include <array>

namespace rzx
{
    using std::array;
}

#else// For freestanding implementation which maybe do not has std::array.

#include "general.hpp"

#include "macro_define.hpp"

namespace rzx
{
    // This simple array is only for this library. 
    // It is not guaranteed to have exactly the same behavior as std::array.
    template<class T, size_t N>
    struct array
    {
        using value_type = T;

        T data[N];

        template<class Self>
        constexpr auto&& operator[](this Self&& self, size_t i)noexcept
        {
            return (self.data[i]);
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

        template<size_t i, derived_from<array> Self>
        friend constexpr decltype(auto) get(Self&& self)noexcept
        {
            return static_cast<fwd_type<decltype(self.data[i]), Self>>(self.data[i]);
        }

        friend constexpr bool operator==(const array&, const array&) = default;
    };

    template<class T>
    struct array<T, 0uz>
    {
        using value_type = T;

        static constexpr size_t size()
        {
            return 0uz;
        }

        constexpr T& operator[](size_t i) noexcept;
        constexpr const T& operator[](size_t i)const noexcept;

        constexpr T* begin()noexcept
        {
            return nullptr;
        }

        constexpr const T* begin()const noexcept
        {
            return nullptr;
        }

        constexpr T* end()noexcept
        {
            return nullptr;
        }

        constexpr const T* end()const noexcept
        {
            return nullptr;
        }

        friend constexpr bool operator==(const array&, const array&) = default;
    };

    template<class...T>
    array(T...) -> array<std::common_type_t<T...>, sizeof...(T)>;
}

template<class T, size_t N>
struct std::tuple_size<rzx::array<T, N>> : std::integral_constant<size_t, N>{};

template<size_t I, class T, size_t N>
struct std::tuple_element<I, rzx::array<T, N>>{
    using type = T;
};

#endif

namespace rzx
{
    template<size_t N, class A>
    constexpr auto array_take(const A& arr)
    {
        using type = A::value_type;
        array<type, N> result;
        for (size_t i = 0; i < N; ++i)
        {
            result[i] = arr[i];
        }
        return result;
    }

    template<size_t N, class A>
    constexpr auto array_drop(const A& arr)
    {
        using type = A::value_type;
        array<type, std::tuple_size_v<A> - N> result;
        for(size_t i = 0; i < std::tuple_size_v<A> - N; ++i)
        {
            result[i] = arr[i + N];
        }
        return result;
    }

    namespace detail
    {
        template<class A1, class A2>
        constexpr auto two_array_cat(const A1& arr1, const A2& arr2)
        {
            using type1 = A1::value_type;
            using type2 = A2::value_type;
            constexpr size_t n1 = std::tuple_size_v<A1>;
            constexpr size_t n2 = std::tuple_size_v<A2>;
            array<std::common_type_t<type1, type2>, n1 + n2> result;

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
    }

    template<class A, class...Rest>
    constexpr auto array_cat(const A& arr, const Rest&...rest)
    {
        if constexpr (sizeof...(rest) == 0)
        {
            return arr;
        }
        else
        {
            return detail::two_array_cat(arr, array_cat(rest...));
        }
    }
}

#include "macro_undef.hpp"
#endif