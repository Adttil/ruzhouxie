#include <ruzhouxie/make.hpp>
#include "test_tool.hpp"

TEST(make, tree_basic)
{

    auto x = 233 | rzx::make<int>;
    auto a = rzx::array{ 1, 3 } | rzx::make<std::tuple<int, int>>;
    
    auto tpl = rzx::tuple{ 1, 3.14f } | rzx::make<std::tuple<int, float>>;

    MAGIC_CHECK(tpl | rzx::child<0>, 1);
    MAGIC_CHECK(tpl | rzx::child<1>, 3.14f);
}

struct X
{
    size_t* copy_count;
    X(size_t& copy_count) : copy_count(&copy_count) {}
    X(const X& x) : copy_count(x.copy_count){ ++*copy_count; };
    X(X&& x) = default;
};

TEST(make, auto_move)
{
    size_t copy_count = 0;
    X{ copy_count } | rzx::refer | rzx::repeat<5> | rzx::make<rzx::array<X, 5>>;
    MAGIC_CHECK(copy_count, 4);
}

struct Foo
{
    template<size_t I>
    constexpr size_t get()const
    {
        return I;
    }
};

template<size_t I>
struct std::tuple_element<I, Foo>
{
    using type = size_t;
};

template<>
struct std::tuple_size<Foo> : std::integral_constant<size_t, 3>{};

constexpr auto foo()
{
    Foo e{};
    auto&& [x, y, z] = e;
    static_assert(std::same_as<decltype(x), size_t>);
    return rzx::array{ x, y, z };
};

TEST(make, for_each)
{
    constexpr auto t = foo();

    struct X
    {
        int x;
        float y;
        const char* z;
    };

    X x{ 1, 3.14f, "hello" };

    x | rzx::for_each([](auto&& x){ std::cout << x << '\n'; });
}