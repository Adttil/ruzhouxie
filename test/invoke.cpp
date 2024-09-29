#include <ruzhouxie/invoke.hpp>
#include <ruzhouxie/make.hpp>
#include "test_tool.hpp"

#include <ruzhouxie/macro_define.hpp>

// TEST(invoke, invoke)
// {
//     int x = 0;
//     auto y = rzx::tuple{ 1, 2 } | rzx::operate(rzx::tuple{ rzx::no_operation, rzx::no_operation });

//     static constexpr auto a = rzx::tuple{ 2, 0.5f };
//     constexpr auto b = a | rzx::transform([](auto x){ return x * x; });

//     constexpr auto ops = rzx::make_tree_of_same_value(rzx::apply_invoke, rzx::tree_shape<rzx::array<size_t, 2>>);
//     auto sops = rzx::detail::simplify_operation_table<ops>(); 

//     using ops_t = decltype(b.operation_table);

//     constexpr auto i = ops_t{};

//     constexpr auto t = rzx::child_count<decltype(b.operation_table)>;
// }

TEST(invoke, transform)
{
    using tttt = rzx::unwrap_t<rzx::view<int>>;
    static constexpr auto a = rzx::tuple{ 2, 0.5f };

    //constexpr auto c = rzx::tuple{ 2, 0.5f } | rzx::repeat<2> | rzx::astrict<rzx::stricture_t::readonly>;
    //constexpr auto b = rzx::tuple{ 2, 0.5f } | rzx::repeat<2> | rzx::astrict<rzx::stricture_t::readonly> | rzx::simplifier<>).data();
    
    //MAGIC_TCHECK(decltype(rzx::zip([](auto x){ return x * x; } | rzx::repeat<2>, rzx::tuple{ 2, 0.5f }) | rzx::child<0> | rzx::simplified_data<>), void);

    constexpr auto d = rzx::apply_invoke(rzx::zip([](auto x){ return x * x; } | rzx::repeat<2>, rzx::tuple{ 2, 0.5f }) | rzx::child<0>);

    //constexpr auto c = rzx::apply_invoke(rzx::zip([](auto x){ return x * x; } | rzx::repeat<2>, rzx::tuple{ 2, 0.5f }) | rzx::child<0>);
    // | rzx::operate(make_tree_of_same_value(rzx::apply_invoke, rzx::tree_shape<rzx::array<size_t, 2>>))
    // ).simplifier<rzx::usage_t::repeatedly>().data()
    ;
    //| rzx::simplified_data<>;

    constexpr auto b = rzx::tuple{ 2, 0.5f } 
     | rzx::transform([](auto x){ return x * x; }) 
    | rzx::make<rzx::tuple<int, float>>
    ;

    MAGIC_CHECK(b | rzx::child<0>, 4);
    MAGIC_CHECK(b | rzx::child<1>, 0.25f);
}

TEST(invoke, zip_transform)
{
    auto a = rzx::tuple{ 2, 0.5f };
    struct { int x; double y; } b{ 3, 0.5 };
    auto c = rzx::zip_transform([](auto x, auto y){ return x * y; }, a, b);

    MAGIC_CHECK(c | rzx::child<0>, 6);
    MAGIC_CHECK(c | rzx::child<1>, 0.25);
}