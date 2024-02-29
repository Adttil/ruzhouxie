#include "test_tool.h"
#include <array>
#include <ruzhouxie\tensor.h>
#include <utility>

namespace rzx = ruzhouxie;

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
    constexpr auto layout = std::array//把[e0, e1]看作[e0, e1, e0, e1, e1]的布局
    {
        std::array{0uz}, std::array{1uz}, std::array{0uz}, std::array{1uz}, std::array{1uz}
    };

    std::array vector{ Tr{}, Tr{} };

    std::puts("==================");
    rzx::vec<5, Tr> result = +(std::move(vector) | rzx::as_ref | rzx::relayout<layout>);
    std::puts("==================");
}