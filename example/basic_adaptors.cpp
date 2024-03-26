#include "test_tool.h"
#include <array>
#include <ruzhouxie\tensor.h>
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
    constexpr auto layout = rzx::tuple{ rzx::array{ 0uz }, rzx::array{ 1uz }, rzx::array{ 0uz } };

    constexpr 
    auto r = rzx::tuple_maker<rzx::tuple<int, double, int>>{}(X{ 1, 4.14}
		| rzx::relayout<layout>
	  	| rzx::transform([](auto i){ return -i; })
	);
    auto [x, y, z] = r;
    std::cout << x << ", " << y << ", " << z << '\n';

    std::tuple trs{ Tr{}, Tr{} };
    constexpr auto layout1 = rzx::tuple//把[x, y]看作[x, x, y, x, y]的布局
    {
        0uz, 1uz, 0uz, 1uz, 1uz
    };

    std::puts("==================");
    auto exp = std::move(trs) | rzx::as_ref | rzx::relayout<layout1>;
    std::array<Tr, 5> result = +(std::move(trs) | rzx::as_ref | rzx::relayout<layout1>);
    std::puts("==================");

    constexpr auto seq = rzx::tuple
    {
        rzx::array{0uz}, rzx::array{1uz}, rzx::array{2uz}, rzx::array{3uz}, rzx::array{4uz}
    };
    auto tape = std::move(exp) | rzx::get_tape<seq>;
    auto [a,b,c,d,e] = tape.sequence;

    MAGIC_SHOW_TYPE(tape.data);

    std::cout << a[0] << ", " << b[0] << ", " << c[0] << ", " << d[0] << ", " << e[0] << '\n';
	
}