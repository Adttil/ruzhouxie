#include <random>
#include <ruzhouxie/quat.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "test_tool.h"

using namespace ruzhouxie;

template<auto>
struct foo_t{};

inline std::mt19937 gen{ 0 };

inline float random() 
{
    static std::uniform_real_distribution<float> dis(-1, 1);
    return dis(gen);
}
TEST(quaternion, rzx)
{
    gen.seed(233);
    mat<3, 3, float> m{ 
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };
    
    for(size_t i = 0; i < 100000000; ++i)
    {
        float w = random();
        float x = random();
        float y = random();
        float z = random();
        mat<3, 3, float> qm = +quat_to_mat3(quat<float>{ w, x, y, z });
        m = +mat_mul(m, qm);
    }
    
    std::cout << "rzx: \n";
    for(auto i : m.base())
    {
        std::cout << i << ", ";
    }
    std::cout << '\n';
}
TEST(quaternion, glm)
{
    gen.seed(233);
    glm::mat3 m{ 1.0f };
    
    for(size_t i = 0; i < 100000000; ++i)
    {
        float w = random();
        float x = random();
        float y = random();
        float z = random();
        m = m * glm::mat3_cast(glm::quat{ w, x, y, z });
    }

    std::cout << "glm: \n";
    for(int i = 0; i < 3; ++i)
    {
        for(int j = 0; j < 3; ++j)
        {
            std::cout << m[j][i] << ", ";
        }
    }
    std::cout << '\n';
}

