#include <ruzhouxie/tree_basic.hpp>
#include "test_tool.hpp"

TEST(tree_basic, make)
{

    auto tpl = rzx::tuple{ 1, 3.14f } | rzx::make<std::tuple<int, float>>;

    MAGIC_CHECK(tpl | rzx::child<0>, 1);
    MAGIC_CHECK(tpl | rzx::child<1>, 3.14f);
}