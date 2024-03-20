#include <ruzhouxie/tensor.h>
#include "test_tool.h"

using namespace ruzhouxie;

TEST(tensor, _)
{
    mat<2, 2> m1 {
        vec<2>{ 1, 2 },
        vec<2>{ 3, 4 }
    };

    mat<2, 2> plus = +(m1 + m1);
    mat<2, 2> mul = +mat_mul(m1, m1);

    MAGIC_CHECK(plus, mat<2, 2>{ vec<2>{ 2, 4 }, vec<2>{ 6, 8 } });
    MAGIC_CHECK(mul, mat<2, 2>{ vec<2>{ 7, 10 }, vec<2>{ 15, 22 } });
}