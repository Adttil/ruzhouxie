#include <ruzhouxie/restrict.hpp>
#include "test_tool.hpp"

TEST(restrict, array)
{
    constexpr auto restrict_table = rzx::tuple{ rzx::stricture_t::readonly, rzx::stricture_t::none };

    auto a = rzx::array{ 1, 2 };

    auto restrict_a = a | rzx::restrict<restrict_table>;

    MAGIC_CHECK(restrict_a | rzx::child<0>, 1);
    MAGIC_CHECK(restrict_a | rzx::child<1>, 2);

    MAGIC_TCHECK(decltype(restrict_a | rzx::child<0>), const int&);
    MAGIC_TCHECK(decltype(restrict_a | rzx::child<1>), int&);
}