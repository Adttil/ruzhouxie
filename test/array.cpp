#include <ruzhouxie/array.hpp>
#include "test_tool.hpp"

using namespace ruzhouxie;

TEST(array, _)
{
    auto arr = array{ 1, 2, 3 };
    MAGIC_CHECK(arr[0], 1);
    MAGIC_CHECK(arr[1], 2);
    MAGIC_CHECK(arr[2], 3);
}