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
    constexpr auto a = detail::concat_array(array{1}, tuple{ array{1} } | child<0>);
    constexpr auto tpl = detail::sequence_add_prefix(tuple{ array{1} }, array{1});
    constexpr auto seq = tree_maker<array<int, 2>>::get_sequence<decltype((t))>();
    t | get_tape<tuple{ array{0uz } }>;
    auto r = array{1,2,3} | to<tuple>();
    //auto r1 = 3 | repeat<3> | to<tuple>();
    //MAGIC_CHECK(r | child<1>, 3);
}