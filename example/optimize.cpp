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

template<auto Info, typename V>
constexpr auto foo(V&& view)
{
    return view;
}

template<typename T>
struct Base
{
    template<typename V>
    constexpr auto operator()(this auto&& self, V&& view)
    requires requires{ foo<self.template custom<V>()>(view); }
    {
        return foo<self.template custom<V>()>(view);
    }
};

struct X : Base<X>
{
    template<typename V>
    static consteval auto custom()
    {
        return 0;
    }
};

int main()
{
    //std::cout << X{}(0);

    std::array<std::array<int, 2>, 2> result = +(233 | rzx::repeat<2> | rzx::transform(neg) |  rzx::repeat<2>);

    for(const auto& arr : result) 
    {
        for(int v : arr) std::cout << v << ' ';
    }
}