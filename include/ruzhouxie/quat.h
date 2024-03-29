#ifndef RUZHOUXIE_QUAT_H
#define RUZHOUXIE_QUAT_H

#include "get.h"
#include "tensor.h"

#include "macro_define.h"

namespace ruzhouxie
{
    RUZHOUXIE_INLINE constexpr auto quat_to_mat3(auto&& q)
	{
        constexpr auto q_layout1 = tuple
        {
            tuple{ tuple{ 1uz, 1uz }, tuple{ 2uz, 2uz }, tuple{ 3uz, 3uz } },
            tuple{ tuple{ 1uz, 2uz }, tuple{ 2uz, 3uz }, tuple{ 3uz, 1uz } },
            tuple{ tuple{ 1uz, 0uz }, tuple{ 2uz, 0uz }, tuple{ 3uz, 0uz } }
        };

        constexpr auto tr_layout1 = array<array<array<size_t, 0uz>, 3uz>, 3uz>{};

        /*
        2xx 2yy 2zz
        2xy 2yz 2zx
        2xw 2yw 2zw
        */
        auto _2_xx_yy_zz_and_xy_yz_zx_and_xw_yw_zw
            = invoke(FWD(q) | relayout<q_layout1>,  [](auto&& arg)
            {
                decltype(auto) product = child<0uz>(arg) * child<1uz>(arg);
                return product + product;
            } | relayout<tr_layout1>);

        constexpr auto i_xx = array{ 0uz, 0uz };
        constexpr auto i_yy = array{ 0uz, 1uz };
        constexpr auto i_zz = array{ 0uz, 2uz };
        constexpr auto i_xy = array{ 1uz, 0uz };
        constexpr auto i_yz = array{ 1uz, 1uz };
        constexpr auto i_zx = array{ 1uz, 2uz };
        constexpr auto i_xw = array{ 2uz, 0uz };
        constexpr auto i_yw = array{ 2uz, 1uz };
        constexpr auto i_zw = array{ 2uz, 2uz };

        constexpr auto q_layout2 = tuple
        {
            tuple{ tuple{ i_yy, i_zz }, tuple{ i_xy, i_zw }, tuple{ i_zx, i_yw } },
            tuple{ tuple{ i_xy, i_zw }, tuple{ i_xx, i_zz }, tuple{ i_yz, i_xw } },
            tuple{ tuple{ i_zx, i_yw }, tuple{ i_yz, i_xw }, tuple{ i_xx, i_yy } }
        };

        auto tr2 = tuple
        {
            [](auto&& arg)
            {
                auto sum = child<0uz>(arg) + child<1uz>(arg);
                return static_cast<decltype(sum)>(1) - sum; 
            },
            [](auto&& arg){ return child<0uz>(arg) + child<1uz>(arg); },
            [](auto&& arg){ return child<0uz>(arg) - child<1uz>(arg); }
        };
        constexpr auto tr_layout2 = tuple
        {
            tuple{ 0uz, 2uz, 1uz },
            tuple{ 1uz, 0uz, 2uz },
            tuple{ 2uz, 1uz, 0uz }
        };

        return invoke(
            FWD(_2_xx_yy_zz_and_xy_yz_zx_and_xw_yw_zw) | relayout<q_layout2>,
            tr2 | relayout<tr_layout2>
        );

        /*
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
        */
	}
}

#include "macro_undef.h"
#endif