#include <ruzhouxie/general.hpp>
#include "test_tool.hpp"

#include <ruzhouxie/macro_define.hpp>

using namespace ruzhouxie;

TEST(specified, same)
{
    MAGIC_CHECK(true, specified<int, int>);
    MAGIC_CHECK(true, specified<const int, int>);
    MAGIC_CHECK(true, specified<int&, int>);
    MAGIC_CHECK(true, specified<const int&, int>);
    MAGIC_CHECK(true, specified<int&&, int>);
    MAGIC_CHECK(true, specified<const int&&, int>);

    MAGIC_CHECK(false, specified<int, const int>);
    MAGIC_CHECK(true, specified<const int, const int>);
    MAGIC_CHECK(false, specified<int&, const int>);
    MAGIC_CHECK(true, specified<const int&, const int>);
    MAGIC_CHECK(false, specified<int&&, const int>);
    MAGIC_CHECK(true, specified<const int&&, const int>);

    MAGIC_CHECK(false, specified<int, int&>);
    MAGIC_CHECK(false, specified<const int, int&>);
    MAGIC_CHECK(true, specified<int&, int&>);
    MAGIC_CHECK(true, specified<const int&, int&>);
    MAGIC_CHECK(false, specified<int&&, int&>);
    MAGIC_CHECK(false, specified<const int&&, int&>);

    MAGIC_CHECK(false, specified<int, const int&>);
    MAGIC_CHECK(false, specified<const int, const int&>);
    MAGIC_CHECK(false, specified<int&, const int&>);
    MAGIC_CHECK(true, specified<const int&, const int&>);
    MAGIC_CHECK(false, specified<int&&, const int&>);
    MAGIC_CHECK(false, specified<const int&&, const int&>);

    MAGIC_CHECK(false, specified<int, int&&>);
    MAGIC_CHECK(false, specified<const int, int&&>);
    MAGIC_CHECK(true, specified<int&, int&&>);
    MAGIC_CHECK(true, specified<const int&, int&&>);
    MAGIC_CHECK(true, specified<int&&, int&&>);
    MAGIC_CHECK(true, specified<const int&&, int&&>);

    MAGIC_CHECK(false, specified<int, const int&&>);
    MAGIC_CHECK(false, specified<const int, const int&>);
    MAGIC_CHECK(false, specified<int&, const int&&>);
    MAGIC_CHECK(true, specified<const int&, const int&&>);
    MAGIC_CHECK(false, specified<int&&, const int&&>);
    MAGIC_CHECK(true, specified<const int&&, const int&&>);
}

TEST(specified, derived)
{
    struct Base{};
    struct Drived : Base{};

    MAGIC_CHECK(true, specified<Drived, Base>);
    MAGIC_CHECK(true, specified<const Drived, Base>);
    MAGIC_CHECK(true, specified<Drived&, Base>);
    MAGIC_CHECK(true, specified<const Drived&, Base>);
    MAGIC_CHECK(true, specified<Drived&&, Base>);
    MAGIC_CHECK(true, specified<const Drived&&, Base>);

    MAGIC_CHECK(false, specified<Drived, const Base>);
    MAGIC_CHECK(true, specified<const Drived, const Base>);
    MAGIC_CHECK(false, specified<Drived&, const Base>);
    MAGIC_CHECK(true, specified<const Drived&, const Base>);
    MAGIC_CHECK(false, specified<Drived&&, const Base>);
    MAGIC_CHECK(true, specified<const Drived&&, const Base>);

    MAGIC_CHECK(false, specified<Drived, Base&>);
    MAGIC_CHECK(false, specified<const Drived, Base&>);
    MAGIC_CHECK(true, specified<Drived&, Base&>);
    MAGIC_CHECK(true, specified<const Drived&, Base&>);
    MAGIC_CHECK(false, specified<Drived&&, Base&>);
    MAGIC_CHECK(false, specified<const Drived&&, Base&>);

    MAGIC_CHECK(false, specified<Drived, const Base&>);
    MAGIC_CHECK(false, specified<const Drived, const Base&>);
    MAGIC_CHECK(false, specified<Drived&, const Base&>);
    MAGIC_CHECK(true, specified<const Drived&, const Base&>);
    MAGIC_CHECK(false, specified<Drived&&, const Base&>);
    MAGIC_CHECK(false, specified<const Drived&&, const Base&>);

    MAGIC_CHECK(false, specified<Drived, Base&&>);
    MAGIC_CHECK(false, specified<const Drived, Base&&>);
    MAGIC_CHECK(true, specified<Drived&, Base&&>);
    MAGIC_CHECK(true, specified<const Drived&, Base&&>);
    MAGIC_CHECK(true, specified<Drived&&, Base&&>);
    MAGIC_CHECK(true, specified<const Drived&&, Base&&>);

    MAGIC_CHECK(false, specified<Drived, const Base&&>);
    MAGIC_CHECK(false, specified<const Drived, const Base&>);
    MAGIC_CHECK(false, specified<Drived&, const Base&&>);
    MAGIC_CHECK(true, specified<const Drived&, const Base&&>);
    MAGIC_CHECK(false, specified<Drived&&, const Base&&>);
    MAGIC_CHECK(true, specified<const Drived&&, const Base&&>);
}

TEST(general, fwd)
{
    struct A
    {
        int a;
        int& b;
        int&& c;
    };

    int i1, i2;

    A a{ i1, i1, std::move(i2) };
    A& r = a;

    MAGIC_TCHECK(decltype(FWD(r, a)), int&);
    MAGIC_TCHECK(decltype(FWD(std::move(a), a)), int&&);
    MAGIC_TCHECK(decltype(FWD(r, b)), int&);
    MAGIC_TCHECK(decltype(FWD(std::move(a), b)), int&);
    MAGIC_TCHECK(decltype(FWD(r, c)), int&);
    MAGIC_TCHECK(decltype(FWD(std::move(a), c)), int&&);
}