#include <ruzhouxie/astrict.hpp>
#include "test_tool.hpp"

TEST(astrict, array)
{
    constexpr auto astrict_table = rzx::tuple{ rzx::stricture_t::readonly, rzx::stricture_t::none };

    auto a = rzx::array{ 1, 2 };

    auto astrict_a = a | rzx::astrict<astrict_table>;

    MAGIC_CHECK(astrict_a | rzx::child<0>, 1);
    MAGIC_CHECK(astrict_a | rzx::child<1>, 2);

    MAGIC_TCHECK(decltype(astrict_a | rzx::child<0>), const int&);
    MAGIC_TCHECK(decltype(astrict_a | rzx::child<1>), int&);
}