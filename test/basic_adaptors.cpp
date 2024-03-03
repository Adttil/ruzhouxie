#include "test_tool.h"
#include <array>
#include <ruzhouxie\transform.h>
#include <print>
//#include <ruzhouxie\constant.h>

namespace rzx = ruzhouxie;

struct X { int a; double b; };
struct Y { double a; double b; };

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
	constexpr auto layout = rzx::tuple{ std::array{ 0uz }, std::array{ 1uz }, std::array{ 0uz } };

	constexpr 
	auto r = rzx::tuple_maker<rzx::tuple<int, double, int>>{}(X{ 1, 4.14}
		| rzx::relayout<layout>
	  	| rzx::transform([](auto i){ return -i; })
	);
	auto [x, y, z] = r;
	std::println("{}, {}, {}", x, y, z);

    std::tuple trs{ Tr{}, Tr{} };
	constexpr auto layout1 = std::array//把[x, y]看作[x, x, y, x, y]的布局
    {
        std::array{0uz}, std::array{1uz}, std::array{0uz}, std::array{1uz}, std::array{1uz}
    };

	std::puts("==================");
    auto result = std::move(trs) | rzx::as_ref | rzx::relayout<layout1> | rzx::make_tree<std::array<Tr, 5>>;
    std::puts("==================");

	// auto r = zip_transform(std::plus<>{}, X{ 1, 3.04 }, std::array{ 232.0, 0.1 }) | make_tree<std::tuple<double, double>>;
	// auto&& [a, b] = r;
	// MAGIC_CHECK(a, 233.0);
	// MAGIC_CHECK(b, 3.14);

	// auto t = zip_transform(std::plus<>{}, X{ 1, 3.04 }, std::array{ 232.0, 0.1 });
	// MAGIC_SHOW_TYPE(t);
	// MAGIC_SHOW_TYPE(t | child<0>);
	// MAGIC_SHOW_TYPE(t | child<1>);
}