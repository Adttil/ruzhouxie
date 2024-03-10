#include <ruzhouxie/enumerate.h>
#include "ruzhouxie/get.h"
#include "test_tool.h"
#include <algorithm>
#include <print>

using namespace ruzhouxie;

int main()
{
    //std::views::transform())
    //constexpr auto r =  tag_invoke_getter{}.get<0uz>(range<0, 2>);
    //constexpr auto r = range<0, 2> | child<0uz>;
    //MAGIC_SHOW_TYPE(r);

    auto r = std::tuple{ 1, 3.14, 3 }
        | enumerate 
        | transform([](auto&& i_v)
            {
                auto&& i = i_v | child<0>;
                auto&& v = i_v | child<1>;
                std::println("{}, {}\n", i.value, v);
                return i_v;
            })
        | to<tuple>()  
        ;
    MAGIC_SHOW_TYPE(r);
}