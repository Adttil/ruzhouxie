#include "test_tool.h"
#include <ruzhouxie\result.h>

namespace rzx = ruzhouxie;

template<auto x>
struct foo {};

struct X { int a; float b; };
struct Y { double a; X b; };

struct Tr
{
	Tr() { std::puts("Tr();"); };
	Tr(const Tr&) { std::puts("Tr(const Tr&);"); }
	Tr(Tr&&)noexcept { std::puts("Tr(Tr&&);"); }
	Tr& operator=(const Tr&) { std::puts("Tr& operator=(const Tr&);"); return *this; }
	Tr& operator=(Tr&&)noexcept { std::puts("Tr& operator=(Tr&&);"); return *this; }
	~Tr()noexcept { std::puts("~Tr();"); }
};

int main()
{
	auto tpl = rzx::tuple{ Tr{}, Tr{} };

	auto tpl_view = std::move(tpl) | rzx::as_ref | rzx::try_tagged;

	auto tpl_tpl_view = rzx::tuple{ tpl_view, tpl_view };

	std::puts("======================");
	auto r = tpl_tpl_view | rzx::to<rzx::tuple>();
	std::puts("======================");
}