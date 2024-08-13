#include <ruzhouxie/relayout.hpp>
#include "test_tool.hpp"

using namespace rzx;

TEST(relayout, array)
{
    constexpr auto layout = rzx::tuple{ rzx::array{ 1 }, rzx::array{ 0 } };

    auto a = rzx::array{ 1, 2 };

    auto relayout_a = a | rzx::relayout<layout>;

    MAGIC_CHECK(relayout_a | child<0>, 2);
    MAGIC_CHECK(relayout_a | child<1>, 1);
}