#include "ruzhouxie/basic_adaptors.h"
#include <array>
#include <cstdio>
#include <functional>
#include <ruzhouxie\tensor.h>

namespace rzx = ruzhouxie;

int main()
{
    constexpr auto layout1 = std::array//把[x]看做[x, x]的layout
    {
        std::array{0uz}, std::array{0uz}
    };
    constexpr auto layout2 = std::array//把[x, y]看做[x, y, x, y]的layout
    {
        std::array{0uz}, std::array{1uz}, std::array{0uz}, std::array{1uz}
    };

    auto input = std::array{ 233 };
    auto neg = [](const auto& x)
    {
        std::puts("neg");
        return -x;
    };

    std::array<int, 4> result = +(input | rzx::relayout<layout1> | rzx::transform(neg) |  rzx::relayout<layout2>);
    for(int x : result) 
    {
        std::cout << x << ' ';
    }
}