#include "test_tool.h"
#include <ruzhouxie\get.h>

namespace rzx = ruzhouxie;
using namespace rzx;

struct X { int x; float& y; double&& z; };

template<auto x>
struct foo {};

int main()
{

	float y{ 3.1f };
	double z{ 1.4 };
	X x{ 233, y, std::move(z) };

	MAGIC_CHECK(x | rzx::child<0>, 233);
	MAGIC_CHECK(x | rzx::child<0>(), 233);
	MAGIC_CHECK(rzx::child<0>(x), 233);
	MAGIC_CHECK(x | rzx::child<1>, 3.1f);
	MAGIC_CHECK(x | rzx::child<2>, 1.4);

	MAGIC_TYPE_CHECK(decltype(x | rzx::child<0>), int&);
	MAGIC_TYPE_CHECK(decltype(std::as_const(x) | rzx::child<0>), const int&);
	MAGIC_TYPE_CHECK(decltype(std::move(x) | rzx::child<0>), int&&);
	MAGIC_TYPE_CHECK(decltype(std::move(std::as_const(x)) | rzx::child<0>), const int&&);

	MAGIC_TYPE_CHECK(decltype(x | rzx::child<1>), float&);
	MAGIC_TYPE_CHECK(decltype(std::as_const(x) | rzx::child<1>), float&);
	MAGIC_TYPE_CHECK(decltype(std::move(x) | rzx::child<1>), float&);
	MAGIC_TYPE_CHECK(decltype(std::move(std::as_const(x)) | rzx::child<1>), float&);

	MAGIC_TYPE_CHECK(decltype(x | rzx::child<2>), double&);
	MAGIC_TYPE_CHECK(decltype(std::as_const(x) | rzx::child<2>), double&);
	MAGIC_TYPE_CHECK(decltype(std::move(x) | rzx::child<2>), double&&);
	MAGIC_TYPE_CHECK(decltype(std::move(std::as_const(x)) | rzx::child<2>), double&&);

	MAGIC_SHOW_TYPE((std::tuple{ std::move(x), std::move(x) } | rzx::child<0, 0>));
	std::cout << int(std::tuple{ std::move(x), std::move(x) } | rzx::child<0uz, 0uz>);
	//std::cout << magic::visualize < foo < rzx::array<int, 3>{-1,2,3} >&> ();
}