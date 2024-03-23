#ifndef RUZHOUXIE_QUAT_H
#define RUZHOUXIE_QUAT_H

#include "get.h"
#include "tensor.h"

#include "macro_define.h"

namespace ruzhouxie
{
    RUZHOUXIE_INLINE constexpr auto quat_to_mat3(auto&& q)
	{
        auto xyz = q | span<1, 3>;
        auto yzx = auto{ xyz } | span<1, 3>;
        //auto zxy = ;
        
        auto _2yy_zz_xx  = auto{ yzx } | transform([](auto&& x){ return 2 * x * x; });
        auto _2zz_xx_yy = auto{ _2yy_zz_xx } | span<1, 3>;

        auto _2xy_yz_zx = zip_transform([](auto&& x, auto&& y){ return 2 * x * y; } ,auto{ xyz }, auto{ yzx });
        auto _2wz_wx_wy = auto{ xyz } | span<2, 3> | transform([w = q | child<0>](auto&& x){ return 2 * x * w; });

        // Result[0][0] = T(1) - T(2) * (qyy + qzz);
        // Result[1][1] = T(1) - T(2) * (qzz + qxx);
        // Result[2][2] = T(1) - T(2) * (qxx + qyy);
        auto main_diagonal = zip_transform([](auto&& x, auto&& y){ return 1 - (x + y); }, auto{_2yy_zz_xx}, auto{_2zz_xx_yy});

        // Result[0][1] = T(2) * (qxy + qwz);
        // Result[1][2] = T(2) * (qyz + qwx);
		// Result[2][0] = T(2) * (qzx + qwy);
        auto l_diagonal = add(auto{ _2xy_yz_zx }, auto{ _2wz_wx_wy });

		// Result[1][0] = T(2) * (qxy - qwz);
		// Result[2][1] = T(2) * (qyz - qwx);
        // Result[0][2] = T(2) * (qzx - qwy);
        auto r_diagonal = sub(auto{ _2xy_yz_zx }, auto{ _2wz_wx_wy });

        constexpr auto layout = tuple
        {
            tuple{ array{ 1uz, 0uz }, array{ 2uz, 0uz }, array{ 0uz, 2uz } },
            tuple{ array{ 0uz, 0uz }, array{ 1uz, 1uz }, array{ 2uz, 1uz } },
            tuple{ array{ 2uz, 2uz }, array{ 0uz, 1uz }, array{ 1uz, 2uz } }
        };

        return tuple{ l_diagonal, main_diagonal, r_diagonal } | relayout<layout>;

		// auto qwx(q.w * q.x);
		// auto qwy(q.w * q.y);
		// auto qwz(q.w * q.z);

		// Result[0][0] = T(1) - T(2) * (qyy +  qzz);
		// Result[0][1] = T(2) * (qxy + qwz);
		// Result[0][2] = T(2) * (qxz - qwy);

		// Result[1][0] = T(2) * (qxy - qwz);
		// Result[1][1] = T(1) - T(2) * (qxx +  qzz);
		// Result[1][2] = T(2) * (qyz + qwx);

		// Result[2][0] = T(2) * (qxz + qwy);
		// Result[2][1] = T(2) * (qyz - qwx);
		// Result[2][2] = T(1) - T(2) * (qxx +  qyy);
		// return Result;

        //auto&&
	    

        // auto _2xx_yy_zz = auto{ xyz } | transform([](auto&& x){ return x * x * 2; });
        // auto _2xy_yz_zx = mul(auto{xyz}, auto{yzx}) | transform([](auto&& x){ return x + x; });
        // auto _2xw_yw_zw = auto{xyz} | transform([&](auto&& x){ return x * (q | child<0>); });

        // return tuple{ _2xx_yy_zz, _2xy_yz_zx, _2xw_yw_zw };
	}
}

#include "macro_undef.h"
#endif