#ifndef RUZHOUXIE_GENERAL_H
#define RUZHOUXIE_GENERAL_H

#include <concepts>
#include <cstddef>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

#include "macro_define.hpp"

#if !defined(__cpp_size_t_suffix) || __cpp_size_t_suffix <= 202006L
constexpr std::size_t operator""uz(unsigned long long x)
{
    return static_cast<std::size_t>(x);
}
#endif

namespace ruzhouxie
{
	using std::size_t;

    namespace rzx = ::ruzhouxie;

    //inline constexpr size_t invalid_index = std::numeric_limits<size_t>::max();

    struct custom_t{};

	//type without cvref.
    //template<class T>
    //concept pure = std::same_as<T, std::remove_cvref_t<T>>;

    //template<class T>
    //using purified = std::remove_cvref_t<T>;

    //template<auto fn>
    //using tag_t = purified<decltype(fn)>;
	
	//completiable type.
    //template<class T>
    //concept concrete = !std::same_as<T, void> && !std::is_abstract_v<T> && !std::is_unbounded_array_v<T>;

	//const object or ref to const object
    //template<class T>
    //concept readonly = std::is_const_v<std::remove_reference_t<T>>;

    template<class T>
    concept aggregated = std::is_aggregate_v<std::remove_cvref_t<T>>;

    //template<class T>
    //concept empty = std::is_empty_v<T>;

    //template<class T>
    //concept empty_aggregated = std::is_empty_v<T> && std::is_aggregate_v<T>;

    //template<class T>
    //concept inheritable = std::is_class_v<T> && !std::is_final_v<T>;

    //template<auto...>
    //concept constevaluable = true;

	template<class T, class U>
    concept derived_from = std::derived_from<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;

    //template<class T, class U>
    //concept is = std::same_as<U, T> || std::derived_from<T, U>;

	//T is type of arbitrarily adding &, &&, const, volatile to TOriginal.
    // template<class T, class TOriginal>
    // concept specified = is<purified<T>, purified<TOriginal>> &&
	// 	(!std::is_reference_v<TOriginal> || std::is_reference_v<T>) &&
	// 	(!std::is_lvalue_reference_v<TOriginal> || std::is_lvalue_reference_v<T>) &&
	// 	(!readonly<TOriginal> || readonly<T>) &&
	// 	(!std::is_volatile_v<std::remove_reference_t<TOriginal>> || std::is_volatile_v<std::remove_reference_t<T>>);

    // template<class T>
    // concept expiringed = std::is_rvalue_reference_v<T>;

	// T is type of non-const r-value reference.
    // template<class T>
    // concept proprietary =
	//     std::is_rvalue_reference_v<T>
	// 	&&
	// 	!std::is_const_v<std::remove_reference_t<T>>;

    // template<class T>
    // T declvalue()noexcept;

    // template<size_t I, class...T>
    // using type_at = std::tuple_element_t<I, std::tuple<T...>>;

	template<class T, class...U>
	using fwd_type = std::conditional_t< (... && std::is_rvalue_reference_v<U&&>),
			    						 std::remove_reference_t<T>&&,
			    						 std::remove_reference_t<T>& >;

    // template<class...T>
    // RUZHOUXIE_INTRINSIC constexpr decltype(auto) fwd(auto&& arg) noexcept
	// {
	// 	return static_cast<fwd_type<decltype(arg), T...>>(arg);
	// }

    // template<class TBase>
    // RUZHOUXIE_INTRINSIC constexpr TBase& as_base(TBase& arg) noexcept
	// {
	//     return arg;
	// }

    // template<class TBase>
    // RUZHOUXIE_INTRINSIC constexpr const TBase& as_base(const TBase& arg) noexcept
	// {
	//     return arg;
	// }

    // template<class TBase>
    // RUZHOUXIE_INTRINSIC constexpr TBase&& as_base(TBase&& arg) noexcept
	// {
	//     return std::move(arg);
	// }

    // template<class TBase>
    // RUZHOUXIE_INTRINSIC constexpr const TBase&& as_base(const TBase&& arg) noexcept
	// {
	//     return std::move(arg);
	// }

    template <class T = bool>
    struct choice_t
	{
	    T strategy{};
	    bool nothrow = false;
	};

	template<class T, std::convertible_to<bool> B>
	choice_t(T&&, B&&) -> choice_t<std::decay<T>>;

    // template<class T>
    // struct wrapper
	// {
	//     T raw_value;

	//     constexpr auto&& value(this auto&& self)noexcept
	// 	{
	// 	    return FWD(self, raw_value);
	// 	}
	// };

	// template<class T>
	// wrapper(T&&) -> wrapper<T>;
}

namespace ruzhouxie
{

    template<size_t I>
    constexpr auto&& arg_at(auto&&...args)noexcept
	{
	    return std::get<I>(std::forward_as_tuple(FWD(args)...));
	}

    // RUZHOUXIE_INLINE constexpr bool equal(auto&& x, auto&& y) 
	//     noexcept(not requires{ FWD(x) == FWD(y); } || requires{ requires noexcept(FWD(x) == FWD(y)); })
	// {
	//     if constexpr(requires{ FWD(x) == FWD(y); })
	// 	{
	// 	    return FWD(x) == FWD(y);
	// 	}
	//     else
	// 	{
	// 	    return false;
	// 	}
	// }
}

#include "macro_undef.hpp"
#endif