#ifndef RUZHOUXIE_TUPLE_H
#define RUZHOUXIE_TUPLE_H
//this tuple is aggregate

#include <type_traits>
#include <utility>
#include <cstddef>

#include "general.hpp"

#include "macro_define.hpp"

namespace ruzhouxie
{
    template<typename...T>
    struct tuple;

    template<typename T, typename...Rest>
    struct tuple<T, Rest...>
	{
	    RUZHOUXIE_MAYBE_EMPTY T              first;
	    RUZHOUXIE_MAYBE_EMPTY tuple<Rest...> rest;

		template<size_t I, specified<tuple> Self> requires (I <= sizeof...(Rest))
	    RUZHOUXIE_INLINE friend constexpr auto&& get(Self&& self) noexcept
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

		//now structure binding is not support get with deducing this in vs17.8.1.

	    // template<size_t I>
		//     requires (I <= sizeof...(Rest))
	    // RUZHOUXIE_INLINE constexpr decltype(auto) get() & noexcept
		// {
		//     if constexpr (I)
		// 	{
		// 	    return rest.template get<I - 1>();
		// 	}
		//     else
		// 	{
		// 	    return (first);
		// 	}
		// }

	    // template<size_t I>
		//     requires (I <= sizeof...(Rest))
	    // RUZHOUXIE_INLINE constexpr decltype(auto) get()const& noexcept
		// {
		//     if constexpr (I)
		// 	{
		// 	    return rest.template get<I - 1>();
		// 	}
		//     else
		// 	{
		// 	    return (first);
		// 	}
		// }

	    // template<size_t I>
		//     requires (I <= sizeof...(Rest))
	    // RUZHOUXIE_INLINE constexpr decltype(auto) get() && noexcept
		// {
		//     if constexpr (I)
		// 	{
		// 	    return std::move(rest).template get<I - 1>();
		// 	}
		//     else
		// 	{
		// 	    return fwd<tuple&&, T>(first);
		// 	}
		// }

	    // template<size_t I>
		//     requires (I <= sizeof...(Rest))
	    // RUZHOUXIE_INLINE constexpr decltype(auto) get()const&& noexcept
		// {
		//     if constexpr (I)
		// 	{
		// 	    return std::move(rest).template get<I - 1>();
		// 	}
		//     else
		// 	{
		// 	    return fwd<const tuple&&, T>(first);
		// 	}
		// }

	    friend constexpr bool operator==(const tuple&, const tuple&) = default;
	};
}

namespace ruzhouxie
{
#include "code_generate/tuple_specialization.code"

	template<typename...T>
    tuple(T...) -> tuple<std::decay_t<T>...>;
}

template<typename...T>
struct std::tuple_size<ruzhouxie::tuple<T...>> : std::integral_constant<std::size_t, sizeof...(T)> {};

template<size_t I, typename...T>
struct std::tuple_element<I, ruzhouxie::tuple<T...>> : std::tuple_element<I, std::tuple<T...>> {};

namespace ruzhouxie
{
	template<typename...Args>
    RUZHOUXIE_INLINE constexpr auto make_tuple(Args&&...args)
	AS_EXPRESSION(tuple<std::decay_t<Args>...>{ FWD(args)... });

	template<typename...Args>
    RUZHOUXIE_INLINE constexpr tuple<Args&&...> fwd_as_tuple(Args&&...args)noexcept
	{
	    return { FWD(args)... };
	};

    template<typename T, typename...Elems>
    RUZHOUXIE_INLINE constexpr auto locate_elem_type(const tuple<Elems...>&, const auto& fn)
	{
	    return locate_type<T, Elems...>(fn);
	}

    template<typename...T, typename V>
    constexpr bool tuple_contain(const tuple<T...>& tpl, const V& value)
	{
	    return [&]<size_t...I>(std::index_sequence<I...>)
		{
		    return (false || ... || rzx::equal(tpl.template get<I>(), value));
		}(std::index_sequence_for<T...>{});
	}

    template<size_t N, typename...Elems>
    constexpr auto tuple_drop(const tuple<Elems...>& tpl)
	{
	    return [&]<size_t...I>(std::index_sequence<I...>)
		{
		    return make_tuple(tpl.template get<I + N>()...);
		}(std::make_index_sequence<sizeof...(Elems) - N>{});
	}
}




#include "macro_undef.hpp"
#endif