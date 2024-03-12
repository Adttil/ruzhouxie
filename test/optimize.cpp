#include "ruzhouxie/tuple.h"
#include <array>
#include <cstdio>
#include <functional>
#include <ruzhouxie\tensor.h>
#include <ranges>
#include <iostream>

namespace rzx = ruzhouxie;

int neg(int x)
{
    std::puts("neg");
    return -x;
};

int main()
{
    constexpr auto mat = std::array
    {
        std::array{1, 2},
        std::array{1, 2}
    };
    constexpr auto m = rzx::mat_mul(mat, mat) | rzx::to();

    std::array<std::array<int, 2>, 2> result = +(233 | rzx::repeat<2> | rzx::transform(neg) |  rzx::repeat<2>);

    for(const auto& arr : result) 
    {
        for(int v : arr) std::cout << v << ' ';
    }
}