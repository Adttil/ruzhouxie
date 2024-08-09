#include <ruzhouxie/adaptor_closure.hpp>
#include "test_tool.hpp"

#include <ruzhouxie/macro_define.hpp>

namespace rzx = ruzhouxie;

struct AddOne : rzx::adaptor_closure<AddOne>
{
    auto operator()(auto x)const
    {
        return x + 1;
    }
};

struct Square : rzx::adaptor_closure<Square>
{
    auto operator()(auto x)const
    {
        return x * x;
    }
};

TEST(adaptor_closure, _)
{
    //using namespace rzx;
    constexpr AddOne add_one{};
    constexpr Square square{};
    constexpr auto mix = add_one | square;

    MAGIC_CHECK(1 | add_one, 2);
    MAGIC_CHECK(1 | add_one | square, 4);
    MAGIC_CHECK(1 | (add_one | square), 4);
    MAGIC_CHECK(1 | mix, 4);
}