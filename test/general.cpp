#include <ruzhouxie/general.h>
#include "test_tool.h"

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