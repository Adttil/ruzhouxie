#include <ruzhouxie/tensor.h>
#include "test_tool.h"

using namespace ruzhouxie;

template<auto>
struct foo_t{};

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
    
    constexpr auto lay = tuple
    {
        array{ 1uz }, array{ 0uz }
    };

    using invec2 = relayout_view<array<float, 2uz>, lay>;

    invec2 iv = array{ 1.0f, 2.0f } | transform(std::negate<>{}) | to<invec2>();

    MAGIC_CHECK(iv | child<0>, -1.0f);
    MAGIC_CHECK(iv | child<1>, -2.0f);
}