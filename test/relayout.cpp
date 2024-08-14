#include <ruzhouxie/relayout.hpp>
#include "test_tool.hpp"

TEST(relayout, array)
{
    constexpr auto layout = rzx::tuple{ rzx::array{ 1 }, rzx::array{ 0 } };

    auto a = rzx::array{ 1, 2 };

    auto relayout_a = a | rzx::relayout<layout>;

    MAGIC_CHECK(relayout_a | rzx::child<0>, 2);
    MAGIC_CHECK(relayout_a | rzx::child<1>, 1);

    MAGIC_TCHECK(decltype(std::move(a) | rzx::refer | rzx::relayout<layout>), rzx::relayout_view<rzx::array<int, 2>&&, layout>);
    MAGIC_TCHECK(decltype(std::move(a) | rzx::relayout<layout>), rzx::relayout_view<rzx::array<int, 2>, layout>);
}

TEST(relayout, repeat)
{
    auto a = rzx::array{ 1, 2 };
    auto b = a | rzx::repeat<2>;
    MAGIC_CHECK(1, b | rzx::child<0, 0>);
    MAGIC_CHECK(2, b | rzx::child<0, 1>);
    MAGIC_CHECK(1, b | rzx::child<1, 0>);
    MAGIC_CHECK(2, b | rzx::child<1, 1>);
}

TEST(relayout, transpose)
{
    auto a = rzx::array{ rzx::array{ 1, 2 }, rzx::array{ 3, 4 } };
    auto b = a | rzx::transpose<>;
    MAGIC_CHECK(1, b | rzx::child<0, 0>);
    MAGIC_CHECK(3, b | rzx::child<0, 1>);
    MAGIC_CHECK(2, b | rzx::child<1, 0>);
    MAGIC_CHECK(4, b | rzx::child<1, 1>);
}