#ifndef RUZHOUXIE_CONSTANT_H
#define RUZHOUXIE_CONSTANT_H

#include "general.hpp"
#include "macro_define.hpp"

namespace rzx
{
    template<auto value>
    struct constant_t : std::integral_constant<decltype(value), value> 
	{
		friend constexpr bool operator==(const constant_t&, const constant_t&) = default;
	};

    template<auto value1, auto value2>
    constexpr auto operator+(constant_t<value1>, constant_t<value2>) 
	{
	    return constant_t<value1 + value2>{};
	}

    template<auto value1, auto value2>
    constexpr auto operator-(constant_t<value1>, constant_t<value2>)
	{
	    return constant_t<value1 - value2>{};
	}

    template<auto value1, auto value2>
    constexpr auto operator*(constant_t<value1>, constant_t<value2>)
	{
	    return constant_t<value1 * value2>{};
	}

    template<auto value1, auto value2>
    constexpr auto operator/(constant_t<value1>, constant_t<value2>)
	{
	    return constant_t<value1 / value2>{};
	}

    template<auto value1, auto value2>
    constexpr auto operator%(constant_t<value1>, constant_t<value2>)
	{
	    return constant_t<value1 % value2>{};
	}
}

// +0
namespace rzx
{
	/*template<auto Zero, typename T> requires requires{ requires (Zero == 0); }
    RUZHOUXIE_INLINE constexpr T operator+(constant_t<Zero>, T&& value)
	{
	    return FWD(value);
	}

    template<auto Zero, typename T> requires requires{ requires (Zero == 0); }
    RUZHOUXIE_INLINE constexpr T operator+(T&& value, constant_t<Zero>)
	{
	    return FWD(value);
	}

    template<auto Zero1, auto Zero2> requires requires{ requires (Zero1 == 0); } && requires{ requires (Zero2 == 0); }
    constexpr auto operator*(constant_t<Zero1>, constant_t<Zero2>)
	{
	    return constant<Zero1 * Zero2>();
	}*/
}

//x0
namespace rzx
{
    template<auto zero> requires requires{ requires zero == 0; }
    constexpr auto operator*(constant_t<zero>, auto&&)
	{
	    return constant_t<zero>{};
	}

    template<auto zero> requires requires{ requires zero == 0; }
    constexpr auto operator*(auto&&, constant_t<zero>)
	{
	    return constant_t<zero>{};
	}
}

//x1
namespace rzx
{
	/*template<auto One, typename T> requires requires{ requires (One == 1); }
    RUZHOUXIE_INLINE constexpr T operator*(constant_t<One>, T&& value)
	{
	    return FWD(value);
	}

    template<auto One, typename T> requires requires{ requires (One == 1); }
    RUZHOUXIE_INLINE constexpr T operator*(T&& value, constant_t<One>)
	{
	    return FWD(value);
	}*/

	/*template<auto One1, auto One2> requires requires{ requires (One1 == 1) && (One2 == 1); }
    constexpr T operator*(T&& value, constant_t<One>)
	{
	    return FWD(value);
	}*/
}

#include "macro_undef.hpp"
#endif