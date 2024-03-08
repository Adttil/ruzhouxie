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
                auto&& [i, v] = i_v;
                std::println("{}, {}\n", i.value, v);
                return i_v;
            })
           
        ;
    MAGIC_SHOW_TYPE(r);
}