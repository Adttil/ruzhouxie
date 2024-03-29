#ifndef RUZHOUXIE_QUAT_H
#define RUZHOUXIE_QUAT_H

#include "get.h"
#include "tensor.h"

#include "macro_define.h"

namespace ruzhouxie
{
    inline constexpr auto quat_to_mat3 = tree_adaptor_closure{ [](auto&& q)
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
                //return 2 * child<0uz>(arg) * child<1uz>(arg);
                auto product = child<0uz>(arg) * child<1uz>(arg);
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
	}};
}

#include "macro_undef.h"
#endif