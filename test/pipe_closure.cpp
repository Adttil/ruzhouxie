#include "ruzhouxie/pipe_closure.h"
#include "test_tool.h"
#include <functional>
#include <ruzhouxie/transform.h>

using namespace ruzhouxie;

int main()
{
    MAGIC_CHECK(3 | pipe_closure{std::negate<>{}}, -3);
    MAGIC_CHECK(3 | pipe_closure{std::negate<>{}} | pipe_closure{std::negate<>{}}, 3);
    MAGIC_CHECK(3 | (pipe_closure{std::negate<>{}} | pipe_closure{std::negate<>{}}), 3);
    //MAGIC_CHECK(3 | transform(std::negate<>{}) | transform(std::negate<>{}), 3);
    //MAGIC_CHECK(3 | (transform(std::negate<>{}) | transform(std::negate<>{})), 3);
}