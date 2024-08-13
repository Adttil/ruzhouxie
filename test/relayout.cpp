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