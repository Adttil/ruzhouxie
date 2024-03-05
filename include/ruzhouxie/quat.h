#ifndef RUZHOUXIE_QUAT_H
#define RUZHOUXIE_QUAT_H

#include "get.h"
#include "tensor.h"

#include "macro_define.h"

namespace ruzhouxie
{
    RUZHOUXIE_INLINE constexpr auto quat_to_mat3(auto&& q)
	{
        //auto&&
		auto xyz = q | span<1, 3>;
        // auto yzx = auto{ xyz } | range<1, 3>;

        // auto _2xx_yy_zz = auto{ xyz } | transform([](auto&& x){ return x * x * 2; });
        // auto _2xy_yz_zx = mul(auto{xyz}, auto{yzx}) | transform([](auto&& x){ return x + x; });
        // auto _2xw_yw_zw = auto{xyz} | transform([&](auto&& x){ return x * (q | child<0>); });

        // return tuple{ _2xx_yy_zz, _2xy_yz_zx, _2xw_yw_zw };
	}
}

#include "macro_undef.h"
#endif