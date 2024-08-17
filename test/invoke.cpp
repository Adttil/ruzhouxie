#include <ruzhouxie/invoke.hpp>
#include <ruzhouxie/make.hpp>
#include "test_tool.hpp"

TEST(invoke, transform)
{
    static constexpr auto a = rzx::tuple{ 2, 0.5f };
    constexpr auto b = a | rzx::transform([](auto x){ return x * x; }) | rzx::make<rzx::tuple<int, float>>;

    MAGIC_CHECK(b | rzx::child<0>, 4);
    MAGIC_CHECK(b | rzx::child<1>, 0.25f);
}

TEST(invoke, zip_transform)
{
    auto a = rzx::tuple{ 2, 0.5f };
    struct { int x; double y; } b{ 3, 0.5 };
    auto c = rzx::zip_transform([](auto x, auto y){ return x * y; }, a, b);

    MAGIC_CHECK(c | rzx::child<0>, 6);
    MAGIC_CHECK(c | rzx::child<1>, 0.25);
}