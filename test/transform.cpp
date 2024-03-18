#include <ruzhouxie/transform.h>
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

TEST(transform, _)
{
    auto vec = array{1, 2};
    transform(vec, std::negate<>{});
    transform(std::negate<>{});
    transform(std::negate<>{})(vec);
    MAGIC_CHECK(vec | transform(std::negate<>{}) | child<1>, -2);
}