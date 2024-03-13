#include <ruzhouxie\get.h>
#include "test_tool.h"

using namespace ruzhouxie;

TEST(child, aggregate)
{
	struct X { int x; float& y; double&& z; };
	float y{ 3.1f };
	double z{ 1.4 };
	X x{ 233, y, std::move(z) };

	MAGIC_CHECK(x | rzx::child<0>, 233);
	MAGIC_CHECK(x | rzx::child<0>(), 233);
	MAGIC_CHECK(rzx::child<0>(x), 233);
	MAGIC_CHECK(x | rzx::child<1>, 3.1f);
	MAGIC_CHECK(x | rzx::child<2>, 1.4);

	MAGIC_TCHECK(decltype(x | rzx::child<0>), int&);
	MAGIC_TCHECK(decltype(std::as_const(x) | rzx::child<0>), const int&);
	MAGIC_TCHECK(decltype(std::move(x) | rzx::child<0>), int&&);
	MAGIC_TCHECK(decltype(std::move(std::as_const(x)) | rzx::child<0>), const int&&);

	MAGIC_TCHECK(decltype(x | rzx::child<1>), float&);
	MAGIC_TCHECK(decltype(std::as_const(x) | rzx::child<1>), float&);
	MAGIC_TCHECK(decltype(std::move(x) | rzx::child<1>), float&);
	MAGIC_TCHECK(decltype(std::move(std::as_const(x)) | rzx::child<1>), float&);

	MAGIC_TCHECK(decltype(x | rzx::child<2>), double&);
	MAGIC_TCHECK(decltype(std::as_const(x) | rzx::child<2>), double&);
	MAGIC_TCHECK(decltype(std::move(x) | rzx::child<2>), double&&);
	MAGIC_TCHECK(decltype(std::move(std::as_const(x)) | rzx::child<2>), double&&);
}