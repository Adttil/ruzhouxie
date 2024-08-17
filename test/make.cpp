#include <ruzhouxie/make.hpp>
#include "test_tool.hpp"

TEST(tree_basic, make)
{

    auto x = 233 | rzx::make<int>;
    auto a = rzx::array{ 1, 3 } | rzx::make<std::tuple<int, int>>;
    
    auto tpl = rzx::tuple{ 1, 3.14f } | rzx::make<std::tuple<int, float>>;

    MAGIC_CHECK(tpl | rzx::child<0>, 1);
    MAGIC_CHECK(tpl | rzx::child<1>, 3.14f);
}