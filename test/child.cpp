#include <ruzhouxie\child.hpp>
#include "test_tool.hpp"

float simpson(auto&& f, float l, float r)
{
    return (f(l) + 4 * f((l + r) / 2) + f(r)) * (r - l) / 6;
}

float adaptive_simpson(auto&& f, float l, float r, float eps)
{
    float s = simpson(f, l, r);
    float mid = (l + r) / 2;
    float sl = simpson(f, l, mid);
    float sr = simpson(f, mid, r);
    float e = (s - sl - sr) / 15;
    
    if(std::fabs(e) <= eps)
    {
        return sl + sr + e;
    }
    return adaptive_simpson(f, l, mid, eps / 2) + adaptive_simpson(f, mid, r, eps / 2);
}

TEST(child, aggregate)
{
    std::cout << "simpson test: " << adaptive_simpson([](float x){ return 10000*x*x*x*x*x; }, 0, 10, 0.00001f) << '\n';

    struct X { int x; float& y; double&& z; };
    float y{ 3.1f };
    double z{ 1.4 };
    X x{ 233, y, std::move(z) };

    MAGIC_CHECK(x | rzx::child<0>, 233);
    MAGIC_CHECK(x | rzx::child<3>, 233);
    MAGIC_CHECK(rzx::child<0>(x), 233);
    MAGIC_CHECK(x | rzx::child<1>, 3.1f);
    MAGIC_CHECK(x | rzx::child<4>, 3.1f);
    MAGIC_CHECK(x | rzx::child<-2>, 3.1f);
    MAGIC_CHECK(x | rzx::child<2>, 1.4);

    MAGIC_TCHECK(decltype(x | rzx::child<0>), int&);
    MAGIC_TCHECK(decltype(std::as_const(x) | rzx::child<0>), const int&);
    MAGIC_TCHECK(decltype(std::move(x) | rzx::child<0>), int&&);
    MAGIC_TCHECK(decltype(std::move(std::as_const(x)) | rzx::child<0>), const int&&);

    MAGIC_TCHECK(decltype(x | rzx::child<1>), float&);
    MAGIC_TCHECK(decltype(std::as_const(x) | rzx::child<1>), float&);
    MAGIC_TCHECK(decltype(std::move(x) | rzx::child<1>), float&);
    MAGIC_TCHECK(decltype(std::move(std::as_const(x)) | rzx::child<1>), float&);

    MAGIC_TCHECK(decltype(x | rzx::child<2>), double&);
    MAGIC_TCHECK(decltype(std::as_const(x) | rzx::child<2>), double&);
    MAGIC_TCHECK(decltype(std::move(x) | rzx::child<2>), double&&);
    MAGIC_TCHECK(decltype(std::move(std::as_const(x)) | rzx::child<2>), double&&);
}