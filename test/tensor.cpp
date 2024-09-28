#include <ruzhouxie/tensor.hpp>
#include <ruzhouxie/make.hpp>
#include <random>
#include "test_tool.hpp"

auto rnd = std::default_random_engine{ 0 };
auto dis = std::uniform_real_distribution<float>{ 0.0f, 1.0f };
size_t N;
int s;

TEST(tensor, mat_mul)
{
    std::cout << "input N: "; std::cin >> N; 
    std::cout << "input seed: "; std::cin >> s; 


    static constexpr auto a = rzx::tuple{
        rzx::tuple{ 1, 2 },
        rzx::tuple{ 3, 4 }
    };

    constexpr auto b = rzx::mat_mul(a, a) | rzx::make<rzx::array<rzx::array<int, 2>, 2>>;

    MAGIC_CHECK(7, b | rzx::child<0, 0>);
    MAGIC_CHECK(10, b | rzx::child<0, 1>);
    MAGIC_CHECK(15, b | rzx::child<1, 0>);
    MAGIC_CHECK(22, b | rzx::child<1, 1>);
}
TEST(tensor, mat_mul_flat)
{
    rnd.seed(s);
    auto a = rzx::tuple{
        rzx::tuple{ 1.0f, 0.0f, 0.0f, 0.0f },
        rzx::tuple{ 0.0f, 1.0f, 0.0f, 0.0f },
        rzx::tuple{ 0.0f, 0.0f, 1.0f, 0.0f },
        rzx::tuple{ 0.0f, 0.0f, 0.0f, 1.0f }
    };
    
    auto b = rzx::tuple{
        rzx::tuple{ dis(rnd), dis(rnd), dis(rnd), dis(rnd) },
        rzx::tuple{ dis(rnd), dis(rnd), dis(rnd), dis(rnd) },
        rzx::tuple{ dis(rnd), dis(rnd), dis(rnd), dis(rnd) },
        rzx::tuple{ 0.0f, 0.0f, 0.0f, 1.0f }
    };
    using rzx::child;
    for(size_t i = 0; i < N; ++i)
    {
        a = rzx::tuple
        {
            rzx::tuple{ 
                child<0, 0>(a) * child<0, 0>(b) + child<0, 1>(a) * child<1, 0>(b) + child<0, 2>(a) * child<2, 0>(b) + child<0, 3>(a) * child<3, 0>(b),
                child<0, 0>(a) * child<0, 1>(b) + child<0, 1>(a) * child<1, 1>(b) + child<0, 2>(a) * child<2, 1>(b) + child<0, 3>(a) * child<3, 1>(b),
                child<0, 0>(a) * child<0, 2>(b) + child<0, 1>(a) * child<1, 2>(b) + child<0, 2>(a) * child<2, 2>(b) + child<0, 3>(a) * child<3, 2>(b),
                child<0, 0>(a) * child<0, 3>(b) + child<0, 1>(a) * child<1, 3>(b) + child<0, 2>(a) * child<2, 3>(b) + child<0, 3>(a) * child<3, 3>(b)
            },
            rzx::tuple{ 
                child<1, 0>(a) * child<0, 0>(b) + child<1, 1>(a) * child<1, 0>(b) + child<1, 2>(a) * child<2, 0>(b) + child<1, 3>(a) * child<3, 0>(b),
                child<1, 0>(a) * child<0, 1>(b) + child<1, 1>(a) * child<1, 1>(b) + child<1, 2>(a) * child<2, 1>(b) + child<1, 3>(a) * child<3, 1>(b),
                child<1, 0>(a) * child<0, 2>(b) + child<1, 1>(a) * child<1, 2>(b) + child<1, 2>(a) * child<2, 2>(b) + child<1, 3>(a) * child<3, 2>(b),
                child<1, 0>(a) * child<0, 3>(b) + child<1, 1>(a) * child<1, 3>(b) + child<1, 2>(a) * child<2, 3>(b) + child<1, 3>(a) * child<3, 3>(b)
            },
            rzx::tuple{ 
                child<2, 0>(a) * child<0, 0>(b) + child<2, 1>(a) * child<1, 0>(b) + child<2, 2>(a) * child<2, 0>(b) + child<2, 3>(a) * child<3, 0>(b),
                child<2, 0>(a) * child<0, 1>(b) + child<2, 1>(a) * child<1, 1>(b) + child<2, 2>(a) * child<2, 1>(b) + child<2, 3>(a) * child<3, 1>(b),
                child<2, 0>(a) * child<0, 2>(b) + child<2, 1>(a) * child<1, 2>(b) + child<2, 2>(a) * child<2, 2>(b) + child<2, 3>(a) * child<3, 2>(b),
                child<2, 0>(a) * child<0, 3>(b) + child<2, 1>(a) * child<1, 3>(b) + child<2, 2>(a) * child<2, 3>(b) + child<2, 3>(a) * child<3, 3>(b)
            },
            rzx::tuple{ 
                child<3, 0>(a) * child<0, 0>(b) + child<3, 1>(a) * child<1, 0>(b) + child<3, 2>(a) * child<2, 0>(b) + child<3, 3>(a) * child<3, 0>(b),
                child<3, 0>(a) * child<0, 1>(b) + child<3, 1>(a) * child<1, 1>(b) + child<3, 2>(a) * child<2, 1>(b) + child<3, 3>(a) * child<3, 1>(b),
                child<3, 0>(a) * child<0, 2>(b) + child<3, 1>(a) * child<1, 2>(b) + child<3, 2>(a) * child<2, 2>(b) + child<3, 3>(a) * child<3, 2>(b),
                child<3, 0>(a) * child<0, 3>(b) + child<3, 1>(a) * child<1, 3>(b) + child<3, 2>(a) * child<2, 3>(b) + child<3, 3>(a) * child<3, 3>(b)
            }
        };
    }

    std::cout << std::format("{},{},{},{}\n{},{},{},{}\n{},{},{},{}\n,{},{},{},{}",
                 child<0, 0>(a), child<0, 1>(a), child<0, 2>(a), child<0, 3>(a),
                 child<1, 0>(a), child<1, 1>(a), child<1, 2>(a), child<1, 3>(a),
                 child<2, 0>(a), child<2, 1>(a), child<2, 2>(a), child<2, 3>(a),
                 child<3, 0>(a), child<3, 1>(a), child<3, 2>(a), child<3, 3>(a)
                 );
    //std::cout << rzx::child<0, 0>(a) << ", " << rzx::child<0, 1>(a) << ", " << rzx::child<1, 0>(a) << ", " << rzx::child<1, 1>(a) << '\n';
}
TEST(tensor, mat_mul_rzx)
{
    rnd.seed(s);
    auto a = rzx::tuple{
        rzx::tuple{ 1.0f, 0.0f, 0.0f, 0.0f },
        rzx::tuple{ 0.0f, 1.0f, 0.0f, 0.0f },
        rzx::tuple{ 0.0f, 0.0f, 1.0f, 0.0f },
        rzx::tuple{ 0.0f, 0.0f, 0.0f, 1.0f }
    };
    
    auto b = rzx::tuple{
        rzx::tuple{ dis(rnd), dis(rnd), dis(rnd), dis(rnd) },
        rzx::tuple{ dis(rnd), dis(rnd), dis(rnd), dis(rnd) },
        rzx::tuple{ dis(rnd), dis(rnd), dis(rnd), dis(rnd) },
        rzx::tuple{ 0.0f, 0.0f, 0.0f, 1.0f }
    };

    for(size_t i = 0; i < N; ++i)
    {
        a = rzx::mat_mul(a, b) | rzx::make<decltype(a)>;
    }

    using rzx::child;
    std::cout << std::format("{},{},{},{}\n{},{},{},{}\n{},{},{},{}\n,{},{},{},{}",
                 child<0, 0>(a), child<0, 1>(a), child<0, 2>(a), child<0, 3>(a),
                 child<1, 0>(a), child<1, 1>(a), child<1, 2>(a), child<1, 3>(a),
                 child<2, 0>(a), child<2, 1>(a), child<2, 2>(a), child<2, 3>(a),
                 child<3, 0>(a), child<3, 1>(a), child<3, 2>(a), child<3, 3>(a)
                 );
    //std::cout << rzx::child<0, 0>(a) << ", " << rzx::child<0, 1>(a) << ", " << rzx::child<1, 0>(a) << ", " << rzx::child<1, 1>(a) << '\n';
}

TEST(tensor, mat_mul_rzx_spec)
{
    rnd.seed(s);
    auto a = rzx::tuple{
        rzx::tuple{ 1.0f, 0.0f, 0.0f, 0.0f },
        rzx::tuple{ 0.0f, 1.0f, 0.0f, 0.0f },
        rzx::tuple{ 0.0f, 0.0f, 1.0f, 0.0f },
        std::tuple{ rzx::constant_t<0>{}, rzx::constant_t<0>{}, rzx::constant_t<0>{}, rzx::constant_t<1>{} }
    };
    
    auto b = rzx::tuple{
        rzx::tuple{ dis(rnd), dis(rnd), dis(rnd), dis(rnd) },
        rzx::tuple{ dis(rnd), dis(rnd), dis(rnd), dis(rnd) },
        rzx::tuple{ dis(rnd), dis(rnd), dis(rnd), dis(rnd) },
        std::tuple{ rzx::constant_t<0>{}, rzx::constant_t<0>{}, rzx::constant_t<0>{}, rzx::constant_t<1>{} }
    };

    for(size_t i = 0; i < N; ++i)
    {
        a = rzx::mat_mul(a, b) | rzx::make<decltype(a)>;
    }

    using rzx::child;
    std::cout << std::format("{},{},{},{}\n{},{},{},{}\n{},{},{},{}\n,{},{},{},{}",
                 child<0, 0>(a), child<0, 1>(a), child<0, 2>(a), child<0, 3>(a),
                 child<1, 0>(a), child<1, 1>(a), child<1, 2>(a), child<1, 3>(a),
                 child<2, 0>(a), child<2, 1>(a), child<2, 2>(a), child<2, 3>(a),
                 child<3, 0>(a).value, child<3, 1>(a).value, child<3, 2>(a).value, child<3, 3>(a).value
                 );
    //std::cout << rzx::child<0, 0>(a) << ", " << rzx::child<0, 1>(a) << ", " << rzx::child<1, 0>(a) << ", " << rzx::child<1, 1>(a) << '\n';
}

// TEST(tensor, triple_mat_mul)
// {
//     static constexpr auto a = rzx::tuple{
//         rzx::tuple{ 1, 2 },
//         rzx::tuple{ 3, 4 }
//     };

//     constexpr auto b = rzx::mat_mul(a, rzx::mat_mul(a, a)) | rzx::make<rzx::array<rzx::array<int, 2>, 2>>;

//     std::cout << std::format("{},{}\n{},{}\n", rzx::child<0, 0>(b), rzx::child<0, 1>(b),  rzx::child<1, 0>(b), rzx::child<1, 1>(b));
// }