#ifndef RUZHOUXIE_TUPLE_H
#define RUZHOUXIE_TUPLE_H
//this tuple is aggregate

#include "general.h"
#include "macro_define.h"
#include "ruzhouxie/macro_define.h"
#include <functional>
#include <type_traits>
#include <utility>

namespace ruzhouxie
{
    template<typename...T>
    struct tuple;

    template<typename T, typename...Rest>
    struct tuple<T, Rest...>
	{
	    RUZHOUXIE_MAYBE_EMPTY T              first;
	    RUZHOUXIE_MAYBE_EMPTY tuple<Rest...> rest;

		/*template<size_t I> requires (I <= sizeof...(Rest))
	    RUZHOUXIE_INLINE constexpr auto&& get(this auto&& self) noexcept
		{
		    if constexpr (I)
			{
			    return FWD(self, rest).get<I - 1>();
			}
		    else
			{
			    return FWD(self, first);
			}
		}*/

		//now structure binding is not support get with deducing this in vs17.8.1.

	    template<size_t I>
		    requires (I <= sizeof...(Rest))
	    RUZHOUXIE_INLINE constexpr decltype(auto) get() & noexcept
		{
		    if constexpr (I)
			{
			    return rest.template get<I - 1>();
			}
		    else
			{
			    return (first);
			}
		}

	    template<size_t I>
		    requires (I <= sizeof...(Rest))
	    RUZHOUXIE_INLINE constexpr decltype(auto) get()const& noexcept
		{
		    if constexpr (I)
			{
			    return rest.template get<I - 1>();
			}
		    else
			{
			    return (first);
			}
		}

	    template<size_t I>
		    requires (I <= sizeof...(Rest))
	    RUZHOUXIE_INLINE constexpr decltype(auto) get() && noexcept
		{
		    if constexpr (I)
			{
			    return std::move(rest).template get<I - 1>();
			}
		    else
			{
			    return fwd<tuple&&, T>(first);
			}
		}

	    template<size_t I>
		    requires (I <= sizeof...(Rest))
	    RUZHOUXIE_INLINE constexpr decltype(auto) get()const&& noexcept
		{
		    if constexpr (I)
			{
			    return std::move(rest).template get<I - 1>();
			}
		    else
			{
			    return fwd<const tuple&&, T>(first);
			}
		}

	    friend constexpr bool operator==(const tuple&, const tuple&) = default;
	};

    template<typename...T>
    tuple(T...) -> tuple<std::decay_t<T>...>;

    template<typename...Args>
    RUZHOUXIE_INLINE constexpr auto make_tuple(Args&&...args)
	    AS_EXPRESSION(tuple<std::decay_t<Args>...>{ FWD(args)... })

    template<typename...Args>
    RUZHOUXIE_INLINE constexpr tuple<Args&&...> fwd_as_tuple(Args&&...args)noexcept
	{
	    return { FWD(args)... };
	}

    template<typename Tpl>
    constexpr auto tuple_cat(Tpl&& tpl)
	{
	    return FWD(tpl);
	}

    template<typename Tpl1, typename Tpl2>
    constexpr auto tuple_cat(Tpl1&& tpl1, Tpl2&& tpl2)
		//noexcept(std::conjunction_v<std::is_nothrow_copy_constructible<Elems1>..., std::is_nothrow_copy_constructible<Elems2>...>)
	{
	    constexpr size_t s1 = std::tuple_size_v<purified<Tpl1>>;
	    constexpr size_t s2 = std::tuple_size_v<purified<Tpl2>>;
	    return[&]<size_t...i, size_t...j>(std::index_sequence<i...>, std::index_sequence<j...>)
		{
		    return tuple<purified<decltype(FWD(tpl1).template get<i>())>..., purified<decltype(FWD(tpl2).template get<j>())>...>
			{
			    FWD(tpl1).template get<i>()..., FWD(tpl2).template get<j>()...
			};
		}(std::make_index_sequence<s1>{}, std::make_index_sequence<s2>{});
	}

    template<typename Tpl1, typename Tpl2, typename...Rest>
    constexpr auto tuple_cat(Tpl1&& tpl1, Tpl2&& tpl2, Rest&&...rest)
		//noexcept(std::conjunction_v<std::is_nothrow_copy_constructible<Elems1>..., std::is_nothrow_copy_constructible<Elems2>...>)
	{
	    return tuple_cat(tuple_cat(FWD(tpl1), FWD(tpl2)), FWD(rest)...);
	}

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
		    return tuple{ tpl.template get<I + N>()... };
		}(std::make_index_sequence<sizeof...(Elems) - N>{});
	}
}


template<typename...T>
struct std::tuple_size<ruzhouxie::tuple<T...>> : std::tuple_size<std::tuple<T...>> {};

template<size_t I, typename...T>
struct std::tuple_element<I, ruzhouxie::tuple<T...>> : std::tuple_element<I, std::tuple<T...>> {};


namespace ruzhouxie
{
#include "generate/tuple_specialization.code"
}

#include "macro_undef.h"
#endif