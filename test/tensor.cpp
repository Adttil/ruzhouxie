#include <ruzhouxie/tensor.hpp>
#include <ruzhouxie/make.hpp>
#include <random>
#include "test_tool.hpp"

auto rnd = std::default_random_engine{ 0 };
auto dis = std::uniform_real_distribution<float>{ -1.0f, 1.0f };
constexpr size_t N = 500000000;

TEST(tensor, mat_mul)
{
    static constexpr auto a = rzx::tuple{
        rzx::tuple{ 1, 2 },
        rzx::tuple{ 3, 4 }
    };

    constexpr auto b = rzx::mat_mul(a, a);// | rzx::make<rzx::array<rzx::array<int, 2>, 2>>;

    MAGIC_CHECK(7, b | rzx::child<0, 0>);
    MAGIC_CHECK(10, b | rzx::child<0, 1>);
    MAGIC_CHECK(15, b | rzx::child<1, 0>);
    MAGIC_CHECK(22, b | rzx::child<1, 1>);
}
TEST(tensor, mat_mul_flat)
{
    rnd.seed(0);
    auto a = rzx::tuple{
        rzx::tuple{ dis(rnd), dis(rnd) },
        rzx::tuple{ dis(rnd), dis(rnd) }
    };

    for(size_t i = 0; i < N; ++i)
    {
        // auto b = rzx::tuple{
        //     rzx::tuple{ dis(rnd), dis(rnd) },
        //     rzx::tuple{ dis(rnd), dis(rnd) }
        // };

        using rzx::child;
        a = rzx::tuple
        {
            rzx::tuple{ child<0, 0>(a) * child<0, 0>(a) + child<0, 1>(a) * child<1, 0>(a), child<0, 0>(a) * child<0, 1>(a) + child<0, 1>(a) * child<1, 1>(a) },
            rzx::tuple{ child<1, 0>(a) * child<0, 0>(a) + child<1, 1>(a) * child<1, 0>(a), child<1, 0>(a) * child<0, 1>(a) + child<1, 1>(a) * child<1, 1>(a) }
        };
        //a = std::move(c);
    }

    std::cout << rzx::child<0, 0>(a) << ", " << rzx::child<0, 1>(a) << ", " << rzx::child<1, 0>(a) << ", " << rzx::child<1, 1>(a) << '\n';
}
TEST(tensor, mat_mul_rzx)
{
    rnd.seed(0);
    auto a = rzx::tuple{
        rzx::tuple{ dis(rnd), dis(rnd) },
        rzx::tuple{ dis(rnd), dis(rnd) }
    };

    for(size_t i = 0; i < N; ++i)
    {
        // auto b = rzx::tuple{
        //     rzx::tuple{ dis(rnd), dis(rnd) },
        //     rzx::tuple{ dis(rnd), dis(rnd) }
        // };

        a = rzx::mat_mul(std::as_const(a), std::as_const(a)) | rzx::make<decltype(a)>;
    }

    std::cout << rzx::child<0, 0>(a) << ", " << rzx::child<0, 1>(a) << ", " << rzx::child<1, 0>(a) << ", " << rzx::child<1, 1>(a) << '\n';
}

