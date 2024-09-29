#include <ruzhouxie/invoke.hpp>
#include <ruzhouxie/make.hpp>
#include "test_tool.hpp"

#include <ruzhouxie/macro_define.hpp>

TEST(invoke, invoke)
{
    int x = 0;
    auto y = rzx::tuple{ 1, 2 } | rzx::operate(rzx::tuple{ rzx::no_operation, rzx::no_operation });

    static constexpr auto a = rzx::tuple{ 2, 0.5f };
    constexpr auto b = a | rzx::transform([](auto x){ return x * x; });

    constexpr auto ops = rzx::make_tree_of_same_value(rzx::apply_invoke, rzx::tree_shape<rzx::array<size_t, 2>>);
    auto sops = rzx::detail::simplify_operation_table<ops>(); 

    using ops_t = decltype(b.operation_table);

    constexpr auto i = ops_t{};

    constexpr auto t = rzx::child_count<decltype(b.operation_table)>;
}

TEST(invoke, transform)
{
    static constexpr auto a = rzx::tuple{ 2, 0.5f };
    constexpr auto b = a | rzx::transform([](auto x){ return x * x; }) 
    //| rzx::sequence
    //| rzx::make<rzx::tuple<int, float>>
    ;

    MAGIC_CHECK(b | rzx::child<0>, 4);
    MAGIC_CHECK(b | rzx::child<1>, 0.25f);
}

// TEST(invoke, zip_transform)
// {
//     auto a = rzx::tuple{ 2, 0.5f };
//     struct { int x; double y; } b{ 3, 0.5 };
//     auto c = rzx::zip_transform([](auto x, auto y){ return x * y; }, a, b);

//     MAGIC_CHECK(c | rzx::child<0>, 6);
//     MAGIC_CHECK(c | rzx::child<1>, 0.25);
// }