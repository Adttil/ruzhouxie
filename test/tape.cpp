#include "test_tool.h"
#include <ruzhouxie/tape.h>

namespace rzx = ruzhouxie;

template<auto x>
struct foo {};

int main()
{
	std::locale::global(std::locale{ ".UTF-8" });

	auto tpl = std::tuple{ 1, 2.0, 3.0f };
	auto tpl2 = rzx::tuple{ 0, tpl, "666" };
	{
		decltype(auto) r = rzx::try_tagged(std::move(tpl) | rzx::as_ref);

		MAGIC_SHOW_TYPE(r);

		MAGIC_TYPE_CHECK(decltype(r | rzx::child<0>), int&);

		MAGIC_TYPE_CHECK(decltype(r | rzx::child<0, rzx::empty_id_set>), int&&);

		MAGIC_TYPE_CHECK(decltype(std::move(r) | rzx::child<0>), int&&);
	}
	/*{
		decltype(auto) r = tpl | rzx::as_ref | rzx::child<0, rzx::empty_id_set>;
		using type = decltype(r);
		MAGIC_TYPE_CHECK(type, const int&);
	}
	{
		decltype(auto) r = std::move(tpl) | rzx::as_ref | rzx::child<0, rzx::id_set{ size_t{0} }>;
		using type = decltype(r);
		MAGIC_TYPE_CHECK(type, const int&);
	}

	{
		decltype(auto) r = std::move(tpl) | rzx::as_ref | rzx::child<0, rzx::empty_id_set>;
		using type = decltype(r);
		MAGIC_TYPE_CHECK(type, int&&);
	}
	{
		decltype(auto) r = std::move(tpl) | rzx::child<0, rzx::empty_id_set> | rzx::as_ref;
		using type = decltype(r);
		MAGIC_TYPE_CHECK(type, int&&);
	}

	decltype(auto) c1 = std::move(tpl) | rzx::as_ref;
	decltype(auto) c2 = std::move(tpl) | rzx::as_ref;

	MAGIC_TYPE_CHECK(decltype(c1), decltype(c2));*/
	//std::cout << '\n' << magic::visualize<decltype(c1)>() << '\n';
	//std::cout << '\n' << magic::visualize<decltype(c2)>() << '\n';

}