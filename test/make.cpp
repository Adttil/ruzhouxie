#include <ruzhouxie/make.hpp>
#include <ruzhouxie/range.hpp>
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
private:
    size_t* copy_count;
public:
    X(size_t& copy_count) : copy_count(&copy_count) {}
    X(const X& x) : copy_count(x.copy_count){ ++*copy_count; };
    X(X&& x) = default;
};

TEST(make, auto_move)
{
    size_t copy_count = 0;
    auto&& x = X{ copy_count };

    decltype(auto) t0 = std::move(x) 
    | rzx::refer 
    | rzx::repeat<2>
    | rzx::relayout<rzx::indexes_of_whole>
    | rzx::astrict<rzx::tuple{ rzx::stricture_t::readonly, rzx::stricture_t::none }>;

    auto&& r = std::move(x) | rzx::refer | rzx::repeat<2>;
    auto&& tt = std::move(r) | rzx::sequence;
    auto&& ttt = std::move(r) 
                | rzx::relayout<rzx::indexes_of_whole>
                | rzx::astrict<rzx::tuple{ rzx::stricture_t::readonly, rzx::stricture_t::none }>;

    auto&& tttt = std::move(x)
                | rzx::refer
                | rzx::relayout_seperate<rzx::tuple{ rzx::indexes_of_whole, rzx::indexes_of_whole }, true>;
                // | rzx::repeat<2>
                // | rzx::astrict<rzx::tuple{ rzx::stricture_t::readonly, rzx::stricture_t::none }>;

    MAGIC_TCHECK(decltype(std::move(tt) | rzx::child<0>), const X&);

    X{ copy_count } | rzx::refer | rzx::repeat<5> | rzx::make<rzx::array<X, 5>>;
    MAGIC_CHECK(copy_count, 4);
}

TEST(make, aggregate)
{
    struct Foo
    {
        X x;
        X y;
        X z;
    };
    MAGIC_CHECK(rzx::aggregate_tree<Foo>, true);
    
    size_t copy_count = 0;
    X{ copy_count } | rzx::refer | rzx::repeat<3> | rzx::make<Foo>;
    MAGIC_CHECK(copy_count, 2);
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

TEST(make, for_each_children)
{
    // struct X
    // {
    //     int x;
    //     float& y;
    //     const char* z;
    // };

    // float f = 3.14f;
    
    // rzx::zip(X{ 1, f, "ok" }, std::tuple{ "hello", 3, 4.5 })
    // | rzx::enumerate 
    // | rzx::for_each_children([](auto i, auto&& v)
    //     {
    //         std::cout << i.value << ' ' << rzx::child<0>(v) << ' ' << rzx::child<1>(v) << '\n';
    //     });
}