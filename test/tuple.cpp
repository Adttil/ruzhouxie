#include <ruzhouxie/tuple.h>
#include "test_tool.h"

using namespace ruzhouxie;

TEST(tuple, get)
{
    auto tpl = tuple{ 1, 3.14, std::string{"wow"} };

	MAGIC_CHECK(tpl.get<0>(), 1);
	MAGIC_CHECK(tpl.get<1>(), 3.14);
	MAGIC_CHECK(tpl.get<2>(), "wow");
	MAGIC_CHECK(tpl, tuple{ 1, 3.14, std::string{"wow"} });
}

TEST(tuple_category, object)
{
	auto tpl = rzx::tuple{ 1 };

	MAGIC_TCHECK(decltype(tpl.get<0>()), int&);
	MAGIC_TCHECK(decltype(std::as_const(tpl).get<0>()), const int&);
	MAGIC_TCHECK(decltype(std::move(tpl).get<0>()), int&&);
	MAGIC_TCHECK(decltype(std::move(std::as_const(tpl)).get<0>()), const int&&);
}

TEST(tuple_category, ref)
{
	int i = 1;
	auto tpl = rzx::tuple<int&>{ i };
	MAGIC_TCHECK(decltype(tpl.get<0>()), int&);
	MAGIC_TCHECK(decltype(std::as_const(tpl).get<0>()), int&);
	MAGIC_TCHECK(decltype(std::move(tpl).get<0>()), int&);
	MAGIC_TCHECK(decltype(std::move(std::as_const(tpl)).get<0>()), int&);
}

TEST(tuple_category, const_ref)
{
	int i = 1;
	auto tpl = rzx::tuple<const int&>{ i };
	MAGIC_TCHECK(decltype(tpl.get<0>()), const int&);
	MAGIC_TCHECK(decltype(std::as_const(tpl).get<0>()), const int&);
	MAGIC_TCHECK(decltype(std::move(tpl).get<0>()), const int&);
	MAGIC_TCHECK(decltype(std::move(std::as_const(tpl)).get<0>()), const int&);
}

TEST(tuple_category, rvalue_ref)
{
	int i = 1;
	auto tpl = rzx::tuple<int&&>{ std::move(i) };
	MAGIC_TCHECK(decltype(tpl.get<0>()), int&);
	MAGIC_TCHECK(decltype(std::as_const(tpl).get<0>()), int&);
	MAGIC_TCHECK(decltype(std::move(tpl).get<0>()), int&&);
	MAGIC_TCHECK(decltype(std::move(std::as_const(tpl)).get<0>()), int&&);
}

TEST(tuple_category, const_rvalue_ref)
{
	int i = 1;
	auto tpl = rzx::tuple<const int&&>{ std::move(i) };
	MAGIC_TCHECK(decltype(tpl.get<0>()), const int&);
	MAGIC_TCHECK(decltype(std::as_const(tpl).get<0>()), const int&);
	MAGIC_TCHECK(decltype(std::move(tpl).get<0>()), const int&&);
	MAGIC_TCHECK(decltype(std::move(std::as_const(tpl)).get<0>()), const int&&);
}

TEST(normal_tuple_category, object)
{
	auto tpl = rzx::tuple{ 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };
	MAGIC_TCHECK(decltype(tpl.get<16>()), int&);
	MAGIC_TCHECK(decltype(std::as_const(tpl).get<16>()), const int&);
	MAGIC_TCHECK(decltype(std::move(tpl).get<16>()), int&&);
	MAGIC_TCHECK(decltype(std::move(std::as_const(tpl)).get<16>()), const int&&);
}

TEST(normal_tuple_category, ref)
{
	int i = 1;
	auto tpl = rzx::fwd_as_tuple(i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i);
	MAGIC_TCHECK(decltype(tpl.get<16>()), int&);
	MAGIC_TCHECK(decltype(std::as_const(tpl).get<16>()), int&);
	MAGIC_TCHECK(decltype(std::move(tpl).get<16>()), int&);
	MAGIC_TCHECK(decltype(std::move(std::as_const(tpl)).get<16>()), int&);
}

TEST(normal_tuple_category, const_ref)
{
	const int i = 1;
	auto tpl = rzx::fwd_as_tuple(i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i);
	MAGIC_TCHECK(decltype(tpl.get<16>()), const int&);
	MAGIC_TCHECK(decltype(std::as_const(tpl).get<16>()), const int&);
	MAGIC_TCHECK(decltype(std::move(tpl).get<16>()), const int&);
	MAGIC_TCHECK(decltype(std::move(std::as_const(tpl)).get<16>()), const int&);
}

TEST(normal_tuple_category, rvalue_ref)
{
	int i = 1;
	auto tpl = rzx::fwd_as_tuple(i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, std::move(i));
	MAGIC_TCHECK(decltype(tpl.get<16>()), int&);
	MAGIC_TCHECK(decltype(std::as_const(tpl).get<16>()), int&);
	MAGIC_TCHECK(decltype(std::move(tpl).get<16>()), int&&);
	MAGIC_TCHECK(decltype(std::move(std::as_const(tpl)).get<16>()), int&&);
}

TEST(normal_tuple_category, const_rvalue_ref)
{
	const int i = 1;
	auto tpl = rzx::fwd_as_tuple(i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, std::move(i));
	MAGIC_TCHECK(decltype(tpl.get<16>()), const int&);
	MAGIC_TCHECK(decltype(std::as_const(tpl).get<16>()), const int&);
	MAGIC_TCHECK(decltype(std::move(tpl).get<16>()), const int&&);
	MAGIC_TCHECK(decltype(std::move(std::as_const(tpl)).get<16>()), const int&&);
}