#include <ruzhouxie/tensor.h>
#include "test_tool.h"

using namespace ruzhouxie;

template<auto>
struct foo_t{};

TEST(tensor, _)
{
    constexpr mat<2, 2> m1 {
        1, 2,
        3, 4
    };

    constexpr mat<2, 2> plus = +(m1 + m1);
    constexpr mat<2, 2> mul = +mat_mul(m1, m1);

    MAGIC_CHECK(plus, mat<2, 2>{ 2, 4, 6, 8 });

    // MAGIC_CHECK(7, mul | child<0, 0>);
    // MAGIC_CHECK(10, mul | child<0, 1>);
    // MAGIC_CHECK(15, mul | child<1, 0>);
    // MAGIC_CHECK(22, mul | child<1, 1>);

    MAGIC_CHECK(mul, mat<2, 2>{ 7, 10, 15, 22 });
    
    constexpr auto lay = tuple
    {
        array{ 1uz }, array{ 0uz }
    };

    using invec2 = relayout_view<array<float, 2uz>, lay>;

    invec2 iv = array{ 1.0f, 2.0f } | transform(std::negate<>{}) | to<invec2>();

    MAGIC_CHECK(iv | child<0>, -1.0f);
    MAGIC_CHECK(iv | child<1>, -2.0f);

    constexpr cmat<2, 2> cm1 {
        1, 3,
        2, 4
    };

    constexpr mat<2, 2> cmul = +mat_mul(cm1, cm1);
    MAGIC_CHECK(cmul, mat<2, 2>{ 7, 10, 15, 22 });
}