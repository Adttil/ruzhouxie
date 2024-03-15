#include "test_tool.h"
#include <array>
#include <ruzhouxie\tensor.h>
#include <utility>

namespace rzx = ruzhouxie;

template <auto x> struct foo_t {};
struct X { int a; double b; };

void fooo()
{
    using namespace ruzhouxie;
    mat<2, 2> m{ 
        vec<2>{ 1.0, 2.0 },
        vec<2>{ 3.0, 4.0 }
    };

    //mat<2, 2> mm = +mat_mul(m, m);
    //std::cout << (mm | child<0uz, 0uz>);
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
    //fooo();
    

    //auto mat = std::array{ X{ 1, 2.0 }, X{ 3, 4.0 } };
    auto mat = rzx::mat<2, 2>{ rzx::vec<2>{ 1.0, 2.0 }, rzx::vec<2>{ 3.0, 4.0 } };
    using mat_t = decltype(mat);
        // rzx::mat<2, 2>{ rzx::array{ 1.0, 2.0 }, rzx::array{ 3.0, 4.0 } };
    auto vec = rzx::array{ 1.0, 2.0 };
    using vec_t = decltype(vec);    

    auto exp = rzx::mat_mul(mat, mat);
    //auto exp0 = rzx::mat_mul(mat0, mat0);
    //auto exp = rzx::dot(vec, vec);
    //std::cout << exp << "  gagaga\n";
    //auto exp = rzx::mat_mul_vec(mat, vec);
    //auto exp = mat | rzx::transpose<>;
    //auto exp = rzx::vec_mul_mat(vec, mat);
    //auto r = exp | rzx::make_tree<vec_t>;
    rzx::mat<2, 2> r = +exp;
    auto r2 = exp | rzx::to<rzx::tuple>();
    //MAGIC_CHECK((mat | rzx::child<0, 0>), 1.0);
    //MAGIC_SHOW_TYPE(exp0);
    MAGIC_SHOW_TYPE(exp);
    MAGIC_SHOW_TYPE(+exp);
    MAGIC_SHOW_TYPE(r);
    
    std::cout << r[0][0] << ", " << r[0][1] << '\n';
    std::cout << r[1][0] << ", " << r[1][1] << '\n';
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