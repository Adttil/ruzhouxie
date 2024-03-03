#include "ruzhouxie/basic_adaptors.h"
#include <array>
#include <cstdio>
#include <functional>
#include <ruzhouxie\tensor.h>
#include <ranges>

namespace rzx = ruzhouxie;

int main()
{
    auto input = std::array{ 233 };
    auto neg = [](const auto& x)
    {
        std::puts("neg");
        return -x;
    };

    std::array<std::array<int, 2>, 2> result = +(233 | rzx::repeat<2> | rzx::transform(neg) |  rzx::repeat<2>);
    for(const auto& arr : result) 
    {
        for(int v : arr) std::cout << v << ' ';
    }
}