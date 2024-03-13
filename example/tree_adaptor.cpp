#include "ruzhouxie/tree_adaptor.h"
#include "test_tool.h"
#include <functional>
#include <ruzhouxie/transform.h>

using namespace ruzhouxie;

int main()
{
    auto neg = tree_adaptor_closure{std::negate<>{}};

    MAGIC_CHECK(3 | neg, -3);
    MAGIC_CHECK(3 | neg | neg, 3);
    MAGIC_CHECK(3 | (neg | neg), 3);
    //MAGIC_CHECK(3 | transform(std::negate<>{}) | transform(std::negate<>{}), 3);
    //MAGIC_CHECK(3 | (transform(std::negate<>{}) | transform(std::negate<>{})), 3);
}