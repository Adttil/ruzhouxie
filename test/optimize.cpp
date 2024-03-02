#include "ruzhouxie/basic_adaptors.h"
#include <array>
#include <cstdio>
#include <functional>
#include <ruzhouxie\tensor.h>

namespace rzx = ruzhouxie;

struct value_t
{
    int value;
    value_t(int value) : value(value){}

    friend value_t operator-(const value_t& l)noexcept
    {
        std::puts("operator-(const value_t&);");
        return value_t{ -l.value };
    }
};

int main()
{
    constexpr auto layout1 = std::array
    {
        std::array{0uz}, std::array{0uz}
    };

    constexpr auto layout2 = std::array
    {
        std::array{0uz}, std::array{1uz}, std::array{0uz}, std::array{1uz}
    };

    auto input = std::array{ value_t{ 233 } };

    std::puts("==================");
    std::array<value_t, 2> result0 = +(input | rzx::relayout<layout1> | rzx::transform(std::negate<>{}));
    std::puts("==================");

    std::puts("==================");
    std::array<value_t, 2> result1 = +(input | rzx::transform(std::negate<>{}) | rzx::relayout<layout1>);
    std::puts("==================");

    std::puts("==================");
    std::array<value_t, 4> result2 = +(input | rzx::relayout<layout1> | rzx::transform(std::negate<>{}) |  rzx::relayout<layout2>);
    std::puts("==================");
}