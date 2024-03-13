#include <gtest/gtest.h>
#include <ruzhouxie/general.h>

using namespace ruzhouxie;

TEST(Specified, SameType)
{
    ASSERT_EQ((specified<int&, int>), true);
}