#include "test_tool.h"
#include <tuple>
#include <ruzhouxie\processer.h>

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

	constexpr std::array index = std::array{ 0uz };

	std::puts("==================");
	auto copy = std::move(tpl) | rzx::make_tree<rzx::tuple<Tr, Tr>>;
	std::puts("==================");
}