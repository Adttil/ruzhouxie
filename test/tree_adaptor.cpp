#include <ruzhouxie/tree_adaptor.h>
#include "test_tool.h"

using namespace ruzhouxie;

TEST(tree_adaptor_closure, struct)
{
    auto neg = tree_adaptor_closure{std::negate<>{}};

    MAGIC_CHECK(3 | neg, -3);
    MAGIC_CHECK(3 | neg | neg, 3);
    MAGIC_CHECK(3 | (neg | neg), 3);
}

TEST(tree_adaptor_closure, no_capture_lmbd)
{
    auto neg = tree_adaptor_closure{ [](auto x){ return -x; } };

    MAGIC_CHECK(3 | neg, -3);
    MAGIC_CHECK(3 | neg | neg, 3);
    MAGIC_CHECK(3 | (neg | neg), 3);
}

TEST(tree_adaptor_closure, value_capture_lmbd)
{
    int zero = 0;
    auto neg = tree_adaptor_closure{ [=](auto x){ return zero - x; } };

    MAGIC_CHECK(3 | neg, -3);
    MAGIC_CHECK(3 | neg | neg, 3);
    MAGIC_CHECK(3 | (neg | neg), 3);
}

TEST(tree_adaptor_closure, ref_capture_lmbd)
{
    int zero = 0;
    auto neg = tree_adaptor_closure{ [&](auto x){ return zero - x; } };

    MAGIC_CHECK(3 | neg, -3);
    MAGIC_CHECK(3 | neg | neg, 3);
    MAGIC_CHECK(3 | (neg | neg), 3);
}