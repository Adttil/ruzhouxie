#include <ruzhouxie/array.hpp>
#include "test_tool.hpp"

using namespace rzx;

TEST(array, basic)
{
    auto arr = array{ 1, 2, 3 };
    
    MAGIC_TCHECK(decltype(arr), array<int, 3>);
    MAGIC_CHECK(arr, array<int, 3>{ 1, 2, 3 });

    MAGIC_CHECK(arr[0], 1);
    MAGIC_CHECK(arr[1], 2);
    MAGIC_CHECK(arr[2], 3);
    
    MAGIC_CHECK(*arr.begin(), 1);
    MAGIC_CHECK(*(arr.end() - 1), 3);
    MAGIC_CHECK(arr.size(), 3);

    MAGIC_TCHECK(decltype(arr[0]), int&);
    MAGIC_TCHECK(decltype(std::as_const(arr)[0]), const int&);
    MAGIC_TCHECK(decltype(std::move(arr)[0]), int&);
    MAGIC_TCHECK(decltype(std::move(std::as_const(arr))[0]), const int&);
}

TEST(array, take)
{
    constexpr auto arr = array{ 1, 2, 3 };
    constexpr auto take = rzx::array_take<2>(arr);
    
    MAGIC_CHECK(take.size(), 2);
    MAGIC_CHECK(take[0], 1);
    MAGIC_CHECK(take[1], 2);
}

TEST(array, drop)
{
    constexpr auto arr = array{ 1, 2, 3 };
    constexpr auto drop = rzx::array_drop<1>(arr);
    
    MAGIC_CHECK(drop.size(), 2);
    MAGIC_CHECK(drop[0], 2);
    MAGIC_CHECK(drop[1], 3);
}

TEST(array, cat)
{
    constexpr auto arr = array{ 1, 2 };
    constexpr auto cat = rzx::array_cat(arr, arr, arr);
    
    MAGIC_CHECK(cat.size(), 6);
    MAGIC_CHECK(cat[0], 1);
    MAGIC_CHECK(cat[1], 2);
    MAGIC_CHECK(cat[2], 1);
    MAGIC_CHECK(cat[3], 2);
    MAGIC_CHECK(cat[4], 1);
    MAGIC_CHECK(cat[5], 2);
}