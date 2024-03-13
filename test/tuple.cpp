#include "test_tool.h"
#include <ruzhouxie/tuple.h>

using namespace ruzhouxie;

TEST(TupleGet, Value)
{
    auto tpl = tuple{ 1, 3.14, std::string{"wow"} };

	MAGIC_CHECK(tpl.get<0>(), 1);
	MAGIC_CHECK(tpl.get<1>(), 3.14);
	MAGIC_CHECK(tpl.get<2>(), "wow");
	MAGIC_CHECK(tpl, tuple{ 1, 3.14, std::string{"wow"} });
}

TEST(TupleGetCategory, Object)
{
	auto tpl = rzx::tuple{ 1 };

	MAGIC_TCHECK(decltype(tpl.get<0>()), int);
	MAGIC_TCHECK(decltype(std::as_const(tpl).get<0>()), const int&);
	MAGIC_TCHECK(decltype(std::move(tpl).get<0>()), int&&);
	MAGIC_TCHECK(decltype(std::move(std::as_const(tpl)).get<0>()), const int&&);
}