#ifndef RUZHOUXIE_TUPLE_HPP
#define RUZHOUXIE_TUPLE_HPP
//this tuple is aggregate

#include <type_traits>
#include <utility>
#include <cstddef>

#include "general.hpp"

#include "macro_define.hpp"

namespace rzx
{
    template<class...T>
    struct tuple;

    template<class T, class...Rest>
    struct tuple<T, Rest...>
	{
	    RUZHOUXIE(no_unique_address) T              first;
	    RUZHOUXIE(no_unique_address) tuple<Rest...> rest;

		template<size_t I, derived_from<tuple> Self> requires (I <= sizeof...(Rest))
	    friend constexpr auto&& get(Self&& self) noexcept
		{
		    if constexpr (I)
			{
			    return get<I - 1>(FWD(self, rest));
			}
		    else
			{
			    return FWD(self, first);
			}
		}

	    friend constexpr bool operator==(const tuple&, const tuple&) = default;
	};
}

namespace rzx
{
#include "code_generate/tuple_specialization.code"

	template<class...T>
    tuple(T...) -> tuple<std::decay_t<T>...>;
}

template<class...T>
struct std::tuple_size<rzx::tuple<T...>> : std::integral_constant<std::size_t, sizeof...(T)> {};

template<size_t I, class...T>
struct std::tuple_element<I, rzx::tuple<T...>> : std::tuple_element<I, std::tuple<T...>> {};

namespace rzx
{
	template<class...Args>
    constexpr auto make_tuple(Args&&...args)
	AS_EXPRESSION(tuple<std::decay_t<Args>...>{ FWD(args)... });

	template<class...Args>
    constexpr tuple<Args&&...> fwd_as_tuple(Args&&...args)noexcept
	{
	    return { FWD(args)... };
	};
}

#include "macro_undef.hpp"
#endif