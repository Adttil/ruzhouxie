#include <ruzhouxie/tensor.h>
#include "test_tool.h"

using namespace ruzhouxie;

template<typename S>
void show_seq(const S& seq, const char* end = "\n")
{
    if constexpr(indicesoid<S>)
    {
        std::cout << "[ ";
        for(auto index : seq)
        {
            std::cout << index << ' ';
        }
        std::cout << ']' << end;
    }
    else
    {
        std::cout << "{ ";
        [&]<size_t...I>(std::index_sequence<I...>)
        {
            (..., show_seq(seq | child<I>, " "));
        }(std::make_index_sequence<child_count<S>>{});
        std::cout << '}' << end;
    }
}

int main()
{
    {
        auto v = array{ 1, 2 };
        //auto f = array{ , std::negate<>{} };
        auto i = v | invoke(std::negate<>{} | repeat<2>) | to();
        std::cout << (i | child<0>) << ", " << (i | child<1>) << '\n';
    }

    // {
    //     auto v = array{ array{ 1, 2 } , array{ 3, 4 } };
    //     //auto f = array{ , std::negate<>{} };
    //     auto i = v | invoke([](const auto& args){ auto&&[a, b] = args; return a + b; } | repeat<2>) | to();
    //     std::cout << (i | child<0>) << ", " << (i | child<1>) << '\n';
    // }

    {
        auto m1 = array{ array{1.0, 2.0}, array{3.0, 4.0} };
        auto m2 = array{ array{1.0, 3.0}, array{2.0, 4.0} };
        auto x = grouped_cartesian(m1 , m1 | transpose<>); 
        MAGIC_SHOW_TYPE(x);

        constexpr auto layout = array{ 
            array{ indices_of_whole_view, indices_of_whole_view }, 
            array{ indices_of_whole_view, indices_of_whole_view }
        };

        auto e = invoke(x, [](auto&& x)->double{ return dot(child<0uz>(x), child<1uz>(x)); } | relayout<layout>);

        MAGIC_SHOW_TYPE(e);
        MAGIC_CHECK(child_count<decltype(e)>, 2);
        MAGIC_CHECK(tensor_shape<decltype(e)>[1], 2);

        MAGIC_CHECK(7, (e | child<0,0>));
        MAGIC_CHECK(10, (e | child<0,1>));
        MAGIC_CHECK(15, (e | child<1,0>));
        MAGIC_CHECK(22, (e | child<1,1>));

        auto rr = e | to();

        MAGIC_SHOW_TYPE(rr);
    }
}