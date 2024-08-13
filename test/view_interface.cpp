#include <ruzhouxie/view_interface.hpp>
#include "test_tool.hpp"

template<auto I>
struct X : rzx::view_interface<X<I>>
{};

template<auto I>
struct Y : rzx::view_interface<X<I>>
{};

TEST(view_interface, viewed)
{
    MAGIC_CHECK(rzx::viewed<X<0>>, true);
    MAGIC_CHECK(rzx::viewed<X<1>>, true);
    MAGIC_CHECK(rzx::viewed<Y<0>>, false);
    MAGIC_CHECK(rzx::viewed<Y<1>>, false);
    MAGIC_CHECK(rzx::viewed<int>, false);
    MAGIC_CHECK(rzx::viewed<rzx::custom_t>, false);
}