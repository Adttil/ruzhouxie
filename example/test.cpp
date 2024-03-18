#include <array>
#include <iostream>

template<auto fn>
using tag_t = std::remove_cvref_t<decltype(fn)>;

namespace foo1_ns
{
    template<size_t I>
    void tag_invoke();

    template<size_t I>
    struct foo1_fn
    {
        auto operator()(auto&& arg)const
        {
            return tag_invoke<I>(foo1_fn<I>{}, arg);
        }
    };
}
template<size_t I>
inline constexpr foo1_ns::foo1_fn<I> foo1{};

namespace foo2_ns
{
    template<auto S>
    void tag_invoke();

    template<auto S>
    struct foo2_fn
    {
        auto operator()(auto&& arg)const
        {
            return tag_invoke<S>(foo2_fn<S>{}, arg);
        }
    };
}
template<auto S>
inline constexpr foo2_ns::foo2_fn<S> foo2{};

namespace ns
{
    struct X
    {
        template<size_t I> requires (I < 3)
        friend auto tag_invoke(tag_t<foo1<I>>, const X&)
        {
            return I;
        };

        template<auto S>
        friend auto tag_invoke(tag_t<foo2<S>>, const X&)
        {
            return S[0];
        };
    };
}

int main()
{
    ns::X x{};
    std::cout << foo1<1>(x);
    std::cout << foo2<std::array{233, 4}>(x);
}