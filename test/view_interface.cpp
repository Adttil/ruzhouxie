#include <ruzhouxie/view_interface.hpp>
#include "test_tool.hpp"

using namespace rzx;

template<auto I>
struct X : rzx::view_interface<X<I>>
{};

template<auto I>
struct Y : rzx::view_interface<X<I>>
{};

TEST(view_interface, viewed)
{
    MAGIC_CHECK(rzx::viewed<X<0>>, true);
    MAGIC_CHECK(rzx::viewed<X<1>>, true);
    MAGIC_CHECK(rzx::viewed<Y<0>>, false);
    MAGIC_CHECK(rzx::viewed<Y<1>>, false);
    MAGIC_CHECK(rzx::viewed<int>, false);
    MAGIC_CHECK(rzx::viewed<rzx::custom_t>, false);
}

template<class T>
using unwrap_r = decltype(unwrap(std::declval<T>()));

TEST(view_interface, unwrap)
{
    MAGIC_TCHECK(unwrap_t<int>, int);
    MAGIC_TCHECK(unwrap_t<int&>, int&);
    MAGIC_TCHECK(unwrap_t<int&&>, int);

    MAGIC_TCHECK(unwrap_t<view<int>>, int);
    MAGIC_TCHECK(unwrap_t<view<int&>>, int&);
    MAGIC_TCHECK(unwrap_t<view<int&&>>, int&&);

    MAGIC_TCHECK(unwrap_t<view<int>&>, int&);
    MAGIC_TCHECK(unwrap_t<view<int&>&>, int&);
    MAGIC_TCHECK(unwrap_t<view<int&&>&>, int&);

    MAGIC_TCHECK(unwrap_t<view<int>&&>, int);
    MAGIC_TCHECK(unwrap_t<view<int&>&&>, int&);
    MAGIC_TCHECK(unwrap_t<view<int&&>&&>, int&&);

    MAGIC_TCHECK(unwrap_r<int>, int&&);
    MAGIC_TCHECK(unwrap_r<int&>, int&);
    MAGIC_TCHECK(unwrap_r<int&&>, int&&);

    MAGIC_TCHECK(unwrap_r<view<int>>, int&&);
    MAGIC_TCHECK(unwrap_r<view<int&>>, int&);
    MAGIC_TCHECK(unwrap_r<view<int&&>>, int&&);

    MAGIC_TCHECK(unwrap_r<view<int>&>, int&);
    MAGIC_TCHECK(unwrap_r<view<int&>&>, int&);
    MAGIC_TCHECK(unwrap_r<view<int&&>&>, int&);

    MAGIC_TCHECK(unwrap_r<view<int>&&>, int&&);
    MAGIC_TCHECK(unwrap_r<view<int&>&&>, int&);
    MAGIC_TCHECK(unwrap_r<view<int&&>&&>, int&&);
}