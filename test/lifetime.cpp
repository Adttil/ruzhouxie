#include "ruzhouxie/relayout.h"
#include "test_tool.h"
#include <array>
#include <functional>
#include <ruzhouxie\transform.h>
#include <print>
//#include <ruzhouxie\constant.h>

namespace rzx = ruzhouxie;

int neg(int x)
{
    std::puts("neg");
    return -x;
};

int main()
{
	//constexpr
    auto r = 233
        | rzx::repeat<3>
        | rzx::range<1, 4>
        | rzx::transform(neg)
        | rzx::repeat<3>
        | rzx::range<3, 2>
        | rzx::transpose<>
        | rzx::to<rzx::tuple>();

}