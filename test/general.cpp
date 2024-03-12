#include <gtest/gtest.h>
#include <ruzhouxie/general.h>

using namespace ruzhouxie;

TEST(Field_Count, NoRef_NoArray)
{
    ASSERT_EQ((specified<int&, int>), true);
}