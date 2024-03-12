#include "test_tool.h"
#include <array>
#include <ruzhouxie\quat.h>
#include <utility>
#include <print>

namespace rzx = ruzhouxie;

int main()
{
    constexpr auto q = std::array{ 1.0, 1.0, 1.0, 1.0};
    // constexpr auto m = ruzhouxie::quat_to_mat3(q) | rzx::to<rzx::tuple>();

    // auto [r0, r1, r2] = m;
    
    // auto [e00, e01, e02] = r0;
    // auto [e10, e11, e12] = r1;
    // auto [e20, e21, e22] = r2;

    // std::println("{},{},{}\n{},{},{}\n{},{},{}", 
    //     e00, e01, e02,
    //     e10, e11, e12,
    //     e20, e21, e22
    // );
}