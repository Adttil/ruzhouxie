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
    MAGIC_CHECK(2, child_count<decltype(3 | repeat<2> | repeat<2>)>);
    3 | get_tape<tuple{ indices_of_whole_view }>;
    auto t = tuple{1,2};
    t | get_tape<tuple{ array{0uz } }>;
    auto r = array{1,2,3} | to<tuple>();
    //auto r1 = 3 | repeat<3> | to<tuple>();
    auto rv = 3 | repeat<3>;
    constexpr auto seq = to<tuple>().get_sequence<decltype((rv))>();
    //constexpr auto tseq = decltype(rv)::mapped_layout<seq>(detail::repeat_layout<3>);
    //auto tape = rv.base() | get_tape<tseq>;
    auto tape1 = tag_invoke<seq>(get_tape<seq>, rv);
    //MAGIC_CHECK(r | child<1>, 3);
}