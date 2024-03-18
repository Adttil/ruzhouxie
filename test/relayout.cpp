#include <ruzhouxie/relayout.h>
#include "test_tool.h"

using namespace ruzhouxie;

struct X
{
    template<size_t I, specified<X> Self> requires (I < 3)
    friend constexpr decltype(auto) tag_invoke(tag_t<child<I>>, Self&& self)
    {
        return I;
    }
};

TEST(relayout, _)
{
    MAGIC_CHECK(3 | repeat<3> | child<1>, 3);
}