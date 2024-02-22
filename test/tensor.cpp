#include "ruzhouxie/adaptors.h"
#include "ruzhouxie/general.h"
#include "ruzhouxie/get.h"
#include "test_tool.h"
#include <array>
#include <ruzhouxie\result.h>
#include <ruzhouxie\tensor.h>

namespace rzx = ruzhouxie;

template <auto x> struct foo {};
struct X { int a; double b; };

void fooo()
{
   using namespace ruzhouxie;
   mat<2, 2> m{ 
       vec<2>{ 1.0, 2.0 },
       vec<2>{ 3.0, 4.0 }
   };

   mat<2, 2> mm = +mat_mul(m, m);
   std::cout << (mm | child<0uz, 0uz>);
   //mat<2, 2> _2m = +(m + m);
   //MAGIC_CHECK((_2m | child<0, 0>), 2.0);

   //auto _3m = +(m * m);
   //MAGIC_CHECK((_3m | child<0, 0>), 3.0);
}

struct Tr
{
    Tr() { std::puts("Tr();"); };
    Tr(const Tr&) { std::puts("Tr(const Tr&);"); }
    Tr(Tr&&)noexcept { std::puts("Tr(Tr&&);"); }
    Tr& operator=(const Tr&) { std::puts("Tr& operator=(const Tr&);"); return *this; }
    Tr& operator=(Tr&&)noexcept { std::puts("Tr& operator=(Tr&&);"); return *this; }
    ~Tr()noexcept { std::puts("~Tr();"); }
};

int main()
{
    fooo();

    constexpr auto layout = std::array
    {
        std::array{0}, std::array{1}, std::array{0}, std::array{1}, std::array{1}
    };

    std::array vector{ Tr{}, Tr{} };

    std::puts("==================");
    rzx::vec<5, Tr> result = +(std::move(vector) | rzx::as_ref | rzx::try_tagged | rzx::relayout<layout>);
    std::puts("==================");

    /*{
        using vec = std::array<int, 2>;
        using mat = std::array<vec, 2>;

        MAGIC_CHECK(rzx::child_count<vec>, 2);
        MAGIC_CHECK(rzx::child_count<mat>, 2);
        MAGIC_CHECK((rzx::child_count<ruzhouxie::child_type<mat, 0>>), 2);

        MAGIC_CHECK(rzx::tensor_rank<vec>, 1);
        MAGIC_CHECK(rzx::tensor_rank<mat>, 2);
        constexpr auto layout = rzx::default_tensor_layout<mat>;
    }*/
    //MAGIC_SHOW_TYPE(foo<layout>{});

    //auto mat = std::array{ X{ 1, 2.0 }, X{ 3, 4.0 } };
    auto mat0 = rzx::array{ rzx::array{ 1.0, 2.0 }, rzx::array{ 3.0, 4.0 } };
    auto mat = rzx::mat<2, 2>{ rzx::vec<2>{ 1.0, 2.0 }, rzx::vec<2>{ 3.0, 4.0 } };
        // rzx::mat<2, 2>{ rzx::array{ 1.0, 2.0 }, rzx::array{ 3.0, 4.0 } };
    auto vec = X{ 1, 2.0 };
    auto exp = rzx::mat_mul(mat, mat);
    //auto exp0 = rzx::mat_mul(mat0, mat0);
    //auto exp = rzx::dot(vec, vec);
    //auto exp = rzx::mat_mul_vec(mat, vec);
    //auto exp = mat | rzx::transpose<>;
    //auto exp = rzx::vec_mul_mat(vec, mat);
    auto r = rzx::to<rzx::tuple>()(exp);
    //MAGIC_CHECK((mat | rzx::child<0, 0>), 1.0);
    //MAGIC_SHOW_TYPE(exp0);
    MAGIC_SHOW_TYPE(exp);
    MAGIC_SHOW_TYPE(r);
    std::cout << 3;
    //std::array<std::array<double, 2>, 2> e{};
    //rzx::mat_mul(r, mat) >> e;

    //std::array<std::array<double, 2>, 2> e = +rzx::mat_mul(r, mat);
    //std::tuple<std::array<double, 2>, std::array<double, 2>> e2 = +rzx::mat_mul(r, mat);

    /*MAGIC_CHECK(
        (ruzhouxie::specified<const ruzhouxie::view<rzx::detail::relayout_view<X, 0>>&, rzx::detail::relayout_view<X, 0>>),
        true);
    MAGIC_CHECK((mat | rzx::component<1, 1>), 2.0);
    MAGIC_CHECK((mat | rzx::component<1, 1> | rzx::child<0>), 2.0);*/
    //MAGIC_CHECK((mat | rzx::component<1, 1> | rzx::child<1>), 4.0);

    //MAGIC_SHOW_TYPE(rzx::tensor_shape<decltype(mat)>);

    //MAGIC_CHECK((mat | rzx::transpose<> | rzx::child<0, 0>), 1);
    //MAGIC_CHECK((mat | rzx::transpose<> | rzx::child<0, 1>), 3);
    //MAGIC_CHECK((mat | rzx::transpose<> | rzx::child<1, 0>), 2.0);
    //MAGIC_CHECK((mat | rzx::transpose<> | rzx::child<1, 1>), 4.0);
}