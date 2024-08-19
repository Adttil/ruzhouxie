#include <ruzhouxie/simplify.hpp>
#include "test_tool.hpp"

template<int N>
struct X
{
    template<auto UsageTable, auto Layout> requires (N <= 0)
    int simplify(rzx::custom_t)const
    {
        return 0;
    }

    template<auto UsageTable, auto Layout> requires (N <= 1)
    friend int simplify(const X&, rzx::custom_t)
    {
        return 1;
    }

    template<auto UsageTable, auto Layout> requires (N <= 2)
    int simplify()const
    {
        return 2;
    }

    template<auto UsageTable, auto Layout> requires (N <= 3)
    friend int simplify(const X&)
    {
        return 3;
    }

    friend bool operator==(const X&, const X&) = default;
};

// TEST(simplify, _)
// {
//     MAGIC_CHECK(X<0>{} | rzx::simplify<>, 0);
//     MAGIC_CHECK(X<1>{} | rzx::simplify<>, 1);
//     MAGIC_CHECK(X<2>{} | rzx::simplify<>, 2);
//     MAGIC_CHECK(X<3>{} | rzx::simplify<>, 3);
//     MAGIC_CHECK(X<4>{} | rzx::simplify<>, X<4>{});
// }