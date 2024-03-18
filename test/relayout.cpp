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
    
    //MAGIC_TCHECK(decltype(3 | repeat<3>), void);
    3 | repeat<3>;
    tag_invoke<0uz>(child<0uz>, view{array{1,2,3}});
    //tag_invoke_getter{}.get<0uz>(3 | repeat<3>);
    //getter_trait<purified<decltype(3 | repeat<3>)>>::choose_default_getter();
    //MAGIC_TCHECK(getter<decltype(3 | repeat<3>)>, tag_invoke_getter);
    //MAGIC_CHECK(3 | repeat<3> | child<1>, 3);
}