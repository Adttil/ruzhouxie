#include "test_tool.h"
#include <array>
#include <ruzhouxie\result.h>
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
    constexpr auto layout = std::array//把[x, y]看作[x, x, y, x, y]的布局
    {
        std::array{0}, std::array{1}, std::array{0}, std::array{1}, std::array{1}
    };

    std::array vector{ Tr{}, Tr{} };

    std::puts("==================");
    rzx::vec<5, Tr> result = +(std::move(vector) | rzx::as_ref | rzx::try_tagged | rzx::relayout<layout>);
    std::puts("==================");
}