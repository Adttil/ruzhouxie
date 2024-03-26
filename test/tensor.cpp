#include <random>
#include <ruzhouxie/tensor.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "test_tool.h"


using namespace ruzhouxie;

inline std::mt19937 gen{ 0 };

inline float random() 
{
    static std::uniform_real_distribution<float> dis(-1, 1);
    return dis(gen);
}

TEST(tensor, _)
{
    constexpr mat<2, 2> m1 {
        1, 2,
        3, 4
    };

    //constexpr mat<2, 2> plus = +(m1 + m1);
    constexpr mat<2, 2> mul = +mat_mul(m1, m1);

    //MAGIC_CHECK(plus, mat<2, 2>{ 2, 4, 6, 8 });

    MAGIC_CHECK((vec<2, float>{1,2}), vec<2, int>{ 1,2 });

    MAGIC_CHECK(false, vec<2, float>{1,2} != vec<2, int>{ 1,2 });

    // MAGIC_CHECK(7, mul | child<0, 0>);
    // MAGIC_CHECK(10, mul | child<0, 1>);
    // MAGIC_CHECK(15, mul | child<1, 0>);
    // MAGIC_CHECK(22, mul | child<1, 1>);

    MAGIC_CHECK(mul, mat<2, 2>{ 7, 10, 15, 22 });
    
    constexpr auto lay = tuple
    {
        array{ 1uz }, array{ 0uz }
    };

    using invec2 = relayout_view<array<float, 2uz>, lay>;

    invec2 iv = array{ 1.0f, 2.0f } | transform(std::negate<>{}) | to<invec2>();

    MAGIC_CHECK(iv | child<0>, -1.0f);
    MAGIC_CHECK(iv | child<1>, -2.0f);

    constexpr cmat<2, 2> cm1 {
        1, 3,
        2, 4
    };

    constexpr mat<2, 2> cmul = +mat_mul(cm1, cm1);
    MAGIC_CHECK(cmul, mat<2, 2>{ 7, 10, 15, 22 });

    constexpr mat<2, 2> emul = +(m1 * m1);
    MAGIC_CHECK(emul, mat<2, 2>{ 1, 4, 9, 16 });
}

TEST(tensor, glm)
{
    gen.seed(233);
    glm::mat4 r{ 1.0f };
    
    for(size_t i = 0; i < 100000000; ++i)
    {
        glm::mat4 m;
        {
            m[0][0] = random();
            m[1][0] = random();
            m[2][0] = random();
            m[3][0] = random();
        
            m[0][1] = random();
            m[1][1] = random();
            m[2][1] = random();
            m[3][1] = random();
        
            m[0][2] = random();
            m[1][2] = random();
            m[2][2] = random();
            m[3][2] = random();
        
            m[0][3] = 0;
            m[1][3] = 0;
            m[2][3] = 0;
            m[3][3] = 1;
        }
        r = r * m;
    }

    std::cout << "glm: \n";
    for(int i = 0; i < 4; ++i)
    {
        for(int j = 0; j < 4; ++j)
        {
            std::cout << r[j][i] << ", ";
        }
    }
    std::cout << '\n';
}

TEST(tensor, rzx)
{
    gen.seed(233);
    mat<4, 4, float> r{ 
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    
    for(size_t i = 0; i < 100000000; ++i)
    {
        mat<4, 4, float> m;
        (m | child<0, 0>) = random();
		(m | child<0, 1>) = random();
		(m | child<0, 2>) = random();
		(m | child<0, 3>) = random();
		(m | child<1, 0>) = random();
		(m | child<1, 1>) = random();
		(m | child<1, 2>) = random();
		(m | child<1, 3>) = random();
		(m | child<2, 0>) = random();
		(m | child<2, 1>) = random();
		(m | child<2, 2>) = random();
		(m | child<2, 3>) = random();

		(m | child<3, 0>) = 0;
		(m | child<3, 1>) = 0;
		(m | child<3, 2>) = 0;
		(m | child<3, 3>) = 1;
        r = +mat_mul(r, m);
    }
    
    std::cout << "rzx: \n";
    for(auto i : r.base())
    {
        std::cout << i << ", ";
    }
    std::cout << '\n';
}

TEST(vec_cross, d3)
{
    vec<3> a{1, 2, 3};
    vec<3> b{3, 4, 5};

    vec<3> r = +cross(a, b);

    MAGIC_CHECK(r, vec<3>{ -2, 4, -2 });
}
