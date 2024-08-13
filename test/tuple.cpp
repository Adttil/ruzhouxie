#include <ruzhouxie/tuple.hpp>
#include "test_tool.hpp"

TEST(tuple, get)
{
    std::tuple<int> t{1};
    auto tpl = rzx::tuple{ 1, 3.14, std::string{"wow"} };

    MAGIC_CHECK(get<0>(tpl), 1);
    MAGIC_CHECK(get<1>(tpl), 3.14);
    MAGIC_CHECK(get<2>(tpl), "wow");
    MAGIC_CHECK(tpl, rzx::tuple{ 1, 3.14, std::string{"wow"} });
}

TEST(tuple, structure_binding)
{
    auto tpl = rzx::tuple{ 1, 3.14, std::string{"wow"} };

    auto&&[x, y, z] = tpl;
    MAGIC_CHECK(x, 1);
    MAGIC_CHECK(y, 3.14);
    MAGIC_CHECK(z, "wow");
    MAGIC_CHECK((rzx::tuple{x, y, z}), rzx::tuple{ 1, 3.14, std::string{"wow"} });
}

TEST(tuple_category, object)
{
    auto tpl = rzx::tuple{ 1 };

    MAGIC_TCHECK(decltype(get<0>(tpl)), int&);
    MAGIC_TCHECK(decltype(get<0>(std::as_const(tpl))), const int&);
    MAGIC_TCHECK(decltype(get<0>(std::move(tpl))), int&&);
    MAGIC_TCHECK(decltype(get<0>(std::move(std::as_const(tpl)))), const int&&);
}

TEST(tuple_category, ref)
{
    int i = 1;
    auto tpl = rzx::tuple<int&>{ i };
    MAGIC_TCHECK(decltype(get<0>(tpl)), int&);
    MAGIC_TCHECK(decltype(get<0>(std::as_const(tpl))), int&);
    MAGIC_TCHECK(decltype(get<0>(std::move(tpl))), int&);
    MAGIC_TCHECK(decltype(get<0>(std::move(std::as_const(tpl)))), int&);
}

TEST(tuple_category, const_ref)
{
    int i = 1;
    auto tpl = rzx::tuple<const int&>{ i };
    MAGIC_TCHECK(decltype(get<0>(tpl)), const int&);
    MAGIC_TCHECK(decltype(get<0>(std::as_const(tpl))), const int&);
    MAGIC_TCHECK(decltype(get<0>(std::move(tpl))), const int&);
    MAGIC_TCHECK(decltype(get<0>(std::move(std::as_const(tpl)))), const int&);
}

TEST(tuple_category, rvalue_ref)
{
    int i = 1;
    auto tpl = rzx::tuple<int&&>{ std::move(i) };
    MAGIC_TCHECK(decltype(get<0>(tpl)), int&);
    MAGIC_TCHECK(decltype(get<0>(std::as_const(tpl))), int&);
    MAGIC_TCHECK(decltype(get<0>(std::move(tpl))), int&&);
    MAGIC_TCHECK(decltype(get<0>(std::move(std::as_const(tpl)))), int&&);
}

TEST(tuple_category, const_rvalue_ref)
{
    int i = 1;
    auto tpl = rzx::tuple<const int&&>{ std::move(i) };
    MAGIC_TCHECK(decltype(get<0>(tpl)), const int&);
    MAGIC_TCHECK(decltype(get<0>(std::as_const(tpl))), const int&);
    MAGIC_TCHECK(decltype(get<0>(std::move(tpl))), const int&&);
    MAGIC_TCHECK(decltype(get<0>(std::move(std::as_const(tpl)))), const int&&);
}

TEST(normal_tuple_category, object)
{
    auto tpl = rzx::tuple{ 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };
    MAGIC_TCHECK(decltype(get<16>(tpl)), int&);
    MAGIC_TCHECK(decltype(get<16>(std::as_const(tpl))), const int&);
    MAGIC_TCHECK(decltype(get<16>(std::move(tpl))), int&&);
    MAGIC_TCHECK(decltype(get<16>(std::move(std::as_const(tpl)))), const int&&);
}

TEST(normal_tuple_category, ref)
{
    int i = 1;
    auto tpl = rzx::fwd_as_tuple(i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i);
    MAGIC_TCHECK(decltype(get<16>(tpl)), int&);
    MAGIC_TCHECK(decltype(get<16>(std::as_const(tpl))), int&);
    MAGIC_TCHECK(decltype(get<16>(std::move(tpl))), int&);
    MAGIC_TCHECK(decltype(get<16>(std::move(std::as_const(tpl)))), int&);
}

TEST(normal_tuple_category, const_ref)
{
    const int i = 1;
    auto tpl = rzx::fwd_as_tuple(i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i);
    MAGIC_TCHECK(decltype(get<16>(tpl)), const int&);
    MAGIC_TCHECK(decltype(get<16>(std::as_const(tpl))), const int&);
    MAGIC_TCHECK(decltype(get<16>(std::move(tpl))), const int&);
    MAGIC_TCHECK(decltype(get<16>(std::move(std::as_const(tpl)))), const int&);
}

TEST(normal_tuple_category, rvalue_ref)
{
    int i = 1;
    auto tpl = rzx::fwd_as_tuple(i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, std::move(i));
    MAGIC_TCHECK(decltype(get<16>(tpl)), int&);
    MAGIC_TCHECK(decltype(get<16>(std::as_const(tpl))), int&);
    MAGIC_TCHECK(decltype(get<16>(std::move(tpl))), int&&);
    MAGIC_TCHECK(decltype(get<16>(std::move(std::as_const(tpl)))), int&&);
}

TEST(normal_tuple_category, const_rvalue_ref)
{
    const int i = 1;
    auto tpl = rzx::fwd_as_tuple(i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, std::move(i));
    MAGIC_TCHECK(decltype(get<16>(tpl)), const int&);
    MAGIC_TCHECK(decltype(get<16>(std::as_const(tpl))), const int&);
    MAGIC_TCHECK(decltype(get<16>(std::move(tpl))), const int&&);
    MAGIC_TCHECK(decltype(get<16>(std::move(std::as_const(tpl)))), const int&&);
}