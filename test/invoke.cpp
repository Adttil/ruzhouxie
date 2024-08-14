#include <ruzhouxie/invoke.hpp>
#include "test_tool.hpp"

TEST(invoke, transform)
{
    auto a = rzx::tuple{ 2, 0.5f };
    auto b = a | rzx::transform([](auto x){ return x * x; });

    MAGIC_CHECK(b | rzx::child<0>, 4);
    MAGIC_CHECK(b | rzx::child<1>, 0.25f);
}