#include <ruzhouxie/tensor.hpp>
#include <ruzhouxie/make.hpp>
#include "test_tool.hpp"

TEST(tensor, mat_mul)
{
    static constexpr auto a = rzx::tuple{
        rzx::tuple{ 1, 2 },
        rzx::tuple{ 3, 4 }
    };

    constexpr auto b = rzx::mat_mul(a, a);// | rzx::make<rzx::array<rzx::array<int, 2>, 2>>;

    MAGIC_CHECK(7, b | rzx::child<0, 0>);
    MAGIC_CHECK(10, b | rzx::child<0, 1>);
    MAGIC_CHECK(15, b | rzx::child<1, 0>);
    MAGIC_CHECK(22, b | rzx::child<1, 1>);
}