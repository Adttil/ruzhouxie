#include <ruzhouxie/invoke.hpp>
#include <ruzhouxie/make.hpp>
#include "test_tool.hpp"

TEST(invoke, transform)
{
    static constexpr auto a = rzx::tuple{ 2, 0.5f };

    auto ff = rzx::tuple{ 2, 0.5f } | rzx::relayout<rzx::indexes_of_whole> | rzx::astrict<rzx::stricture_t::none>;
    MAGIC_TCHECK(decltype(ff), decltype(rzx::view{ rzx::tuple{ 2, 0.5f } }));

    constexpr auto b = a | rzx::transform([](auto x){ return x * x; }) | rzx::make<rzx::tuple<int, float>>;

    

    MAGIC_CHECK(b | rzx::child<0>, 4);
    MAGIC_CHECK(b | rzx::child<1>, 0.25f);
    
    auto f = [](auto x){ return x * x; };
    auto base = rzx::zip(f | rzx::repeat<2>, a);
    auto t = rzx::tuple{ f | rzx::repeat<2>, rzx::tuple{ 1, 2 } }
            | rzx::transpose<> 
            | rzx::invoke(rzx::tuple{ rzx::leaf_tag_t{}, rzx::leaf_tag_t{} }) 
            | rzx::seperate;
}

TEST(invoke, zip_transform)
{
    auto a = rzx::tuple{ 2, 0.5f };
    struct { int x; double y; } b{ 3, 0.5 };
    auto c = rzx::zip_transform([](auto x, auto y){ return x * y; }, a, b);

    MAGIC_CHECK(c | rzx::child<0>, 6);
    MAGIC_CHECK(c | rzx::child<1>, 0.25);
}