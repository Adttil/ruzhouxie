#include "ruzhouxie/enumerate.h"
#include "ruzhouxie/get.h"
#include "test_tool.h"
#include <print>

using namespace ruzhouxie;

int main()
{
    constexpr auto r =  tag_invoke_getter{}.get<0>(range<0, 2>);
    //constexpr auto r = range<0, 2> | child<0>;
    MAGIC_SHOW_TYPE(r);
    // auto r = zip(std::tuple{ 1, 3.14 }, range<0, 2>) 
    //     //| enumerate 
    //     | transform([](auto&& i_v)
    //         {
    //             auto&& [i, v] = i_v;
    //             std::println("{}, {}\n", i, v);
    //             return i_v;
    //         })
    //     | to<tuple>()    
    //     ;
}