#include "test_tool.h"
#include <ruzhouxie\tuple.h>

namespace rzx = ruzhouxie;

int main()
{
	{
		auto tpl = rzx::tuple{ 1, 3.14, std::string{"wow"} };

		MAGIC_CHECK(tpl.get<0>(), 1);
		MAGIC_CHECK(tpl.get<1>(), 3.14);
		MAGIC_CHECK(tpl.get<2>(), "wow");
		MAGIC_CHECK(tpl, (rzx::tuple{ 1, 3.14, std::string{"wow"} }));
	}

	//for specialization
	{
		auto tpl = rzx::tuple{ 1 };

		MAGIC_TYPE_CHECK(decltype(tpl.get<0>()), int&);
		MAGIC_TYPE_CHECK(decltype(std::as_const(tpl).get<0>()), const int&);
		MAGIC_TYPE_CHECK(decltype(std::move(tpl).get<0>()), int&&);
		MAGIC_TYPE_CHECK(decltype(std::move(std::as_const(tpl)).get<0>()), const int&&);
	}
	{
		int i = 1;
		auto tpl = rzx::tuple<int&>{ i };

		MAGIC_TYPE_CHECK(decltype(tpl.get<0>()), int&);
		MAGIC_TYPE_CHECK(decltype(std::as_const(tpl).get<0>()), int&);
		MAGIC_TYPE_CHECK(decltype(std::move(tpl).get<0>()), int&);
		MAGIC_TYPE_CHECK(decltype(std::move(std::as_const(tpl)).get<0>()), int&);
	}
	{
		int i = 1;
		auto tpl = rzx::tuple<const int&>{ i };

		MAGIC_TYPE_CHECK(decltype(tpl.get<0>()), const int&);
		MAGIC_TYPE_CHECK(decltype(std::as_const(tpl).get<0>()), const int&);
		MAGIC_TYPE_CHECK(decltype(std::move(tpl).get<0>()), const int&);
		MAGIC_TYPE_CHECK(decltype(std::move(std::as_const(tpl)).get<0>()), const int&);
	}
	{
		int i = 1;
		auto tpl = rzx::tuple<int&&>{ std::move(i) };

		MAGIC_TYPE_CHECK(decltype(tpl.get<0>()), int&);
		MAGIC_TYPE_CHECK(decltype(std::as_const(tpl).get<0>()), int&);
		MAGIC_TYPE_CHECK(decltype(std::move(tpl).get<0>()), int&&);
		MAGIC_TYPE_CHECK(decltype(std::move(std::as_const(tpl)).get<0>()), int&&);
	}
	{
		int i = 1;
		auto tpl = rzx::tuple<const int&&>{ std::move(i) };

		MAGIC_TYPE_CHECK(decltype(tpl.get<0>()), const int&);
		MAGIC_TYPE_CHECK(decltype(std::as_const(tpl).get<0>()), const int&);
		MAGIC_TYPE_CHECK(decltype(std::move(tpl).get<0>()), const int&&);
		MAGIC_TYPE_CHECK(decltype(std::move(std::as_const(tpl)).get<0>()), const int&&);
	}

	//for normal
	{
		auto tpl = rzx::tuple{ 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };

		MAGIC_TYPE_CHECK(decltype(tpl.get<16>()), int&);
		MAGIC_TYPE_CHECK(decltype(std::as_const(tpl).get<16>()), const int&);
		MAGIC_TYPE_CHECK(decltype(std::move(tpl).get<16>()), int&&);
		MAGIC_TYPE_CHECK(decltype(std::move(std::as_const(tpl)).get<16>()), const int&&);
	}
	{
		int i = 1;
		auto tpl = rzx::fwd_as_tuple(i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i);

		MAGIC_TYPE_CHECK(decltype(tpl.get<16>()), int&);
		MAGIC_TYPE_CHECK(decltype(std::as_const(tpl).get<16>()), int&);
		MAGIC_TYPE_CHECK(decltype(std::move(tpl).get<16>()), int&);
		MAGIC_TYPE_CHECK(decltype(std::move(std::as_const(tpl)).get<16>()), int&);
	}
	{
		const int i = 1;
		auto tpl = rzx::fwd_as_tuple(i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i);

		MAGIC_TYPE_CHECK(decltype(tpl.get<16>()), const int&);
		MAGIC_TYPE_CHECK(decltype(std::as_const(tpl).get<16>()), const int&);
		MAGIC_TYPE_CHECK(decltype(std::move(tpl).get<16>()), const int&);
		MAGIC_TYPE_CHECK(decltype(std::move(std::as_const(tpl)).get<16>()), const int&);
	}
	{
		int i = 1;
		auto tpl = rzx::fwd_as_tuple(i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, std::move(i));

		MAGIC_TYPE_CHECK(decltype(tpl.get<16>()), int&);
		MAGIC_TYPE_CHECK(decltype(std::as_const(tpl).get<16>()), int&);
		MAGIC_TYPE_CHECK(decltype(std::move(tpl).get<16>()), int&&);
		MAGIC_TYPE_CHECK(decltype(std::move(std::as_const(tpl)).get<16>()), int&&);
	}
	{
		const int i = 1;
		auto tpl = rzx::fwd_as_tuple(i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, std::move(i));

		MAGIC_TYPE_CHECK(decltype(tpl.get<16>()), const int&);
		MAGIC_TYPE_CHECK(decltype(std::as_const(tpl).get<16>()), const int&);
		MAGIC_TYPE_CHECK(decltype(std::move(tpl).get<16>()), const int&&);
		MAGIC_TYPE_CHECK(decltype(std::move(std::as_const(tpl)).get<16>()), const int&&);
	}
}