#include <ruzhouxie/invoke.h>
#include "test_tool.h"

using namespace ruzhouxie;

int main()
{
    auto v = array{ 1, 2 };
    //auto f = array{ , std::negate<>{} };
    auto i = invoke(v, std::negate<>{} | repeat<2>) | to();
    std::cout << (i | child<0>) << (i | child<1>);
}