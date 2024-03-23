#include "test_tool.h"
#include <ruzhouxie\constant.h>
#include <ruzhouxie\tensor.h>
#include <glm\glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <random>
#include <vector>
#include <chrono>

using namespace ruzhouxie;

#define TEST_INT 0

using value_t = std::conditional_t<TEST_INT, int, float>;

using rzxmat = tuple
<
    tuple<value_t, value_t, value_t, value_t>,
    tuple<value_t, value_t, value_t, value_t>,
    tuple<value_t, value_t, value_t, value_t>,
    tuple<value_t, value_t, value_t, value_t>
	/*std::conditional_t<TEST_INT,*/
		//tuple<constant_t<0>, constant_t<0>, constant_t<0>, constant_t<1>>
	/*,
	    tuple<constant_t<0.0f>, constant_t<0.0f>, constant_t<0.0f>, constant_t<1.0f>>
	>*/
>;

using rzxmat2 = tuple
<
    tuple<value_t, value_t, value_t, value_t>,
    tuple<value_t, value_t, value_t, value_t>,
    tuple<value_t, value_t, value_t, value_t>,
    tuple<constant_t<0>, constant_t<0>, constant_t<0>, constant_t<1>>
>;


struct timer
{
    decltype(std::chrono::system_clock::now()) start;

    timer()
	{
	    std::puts("=======================================");
	    start = std::chrono::system_clock::now();
	}

	~timer()
	{
	    auto end = std::chrono::system_clock::now();
	    auto dur = end - start;
	    std::cout << '{' << (dur.count() * double(decltype(dur)::period().num) / decltype(dur)::period().den) << "}====================================";
	}
};

std::mt19937 gen{ 0 };
size_t N;

value_t random() 
{
    static std::conditional_t<TEST_INT, std::uniform_int_distribution<>, std::uniform_real_distribution<value_t>> 
	    dis(-1, 1);
    return dis(gen);
}

value_t zero;
value_t one;

void glm_test();
void rzx_test();
void rzx2_test();

int main()
{
    unsigned int seed;
    std::cin >> zero >> one;
    std::cin >> seed;
    std::cin >> N;

    gen.seed(seed);
    glm_test();
	
    gen.seed(seed);
    rzx_test();

    gen.seed(seed);
    rzx2_test();
}

void glm_test()
{
    using namespace glm;
	
    using matrix = std::conditional_t<TEST_INT, imat4x4, mat4x4>;

    matrix m{};
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

	    m[0][3] = ::zero;
	    m[1][3] = ::zero;
	    m[2][3] = ::zero;
	    m[3][3] = ::one;
	}

	{
		glm::quat q1{glm::radians(glm::vec3(0.0f, 0.0f, 90.0f))};
		glm::mat4_cast(q1);
	    std::cout << "glm\n";
	    timer t{};
	    for (size_t i = 0; i < N; ++i)
		{
            m = m * m;
			//data[i - 2] = data[i - 1] * data[i];
			// auto&& r = m * m[3];
			// m[0][0] = r[0];
			// m[1][0] = r[1];
			// m[2][0] = r[2];
			// m[3][0] = r[3];
			//m = m * m;
		}
	}

    std::cout << m[0][0] << ", " << m[2][2] << '\n';
}

//RUZHOUXIE_INLINE constexpr rzxmat rzx_mul(const rzxmat& l, const rzxmat& r)
//{
//    return mat_mul(l, r) | to<tuple>();
//}

void rzx_test()
{
    rzxmat m{};

	{
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

		(m | child<3, 0>) = zero;
		(m | child<3, 1>) = zero;
		(m | child<3, 2>) = zero;
		(m | child<3, 3>) = one;
	}

	{
	    std::cout << "rzx\n";
	    timer t{};
	    for (size_t i = 0; i < N; ++i)
		{
			//m = mat_mul(m, m);

			//[[msvc::flatten]]
		    m = +mat_mul(m, m);

			//m | child<0> = mat_mul_vec(m, m | component<3, 1>)/* | to<tuple>()*/;
			/*m | child<0, 3> = r | child<0>;
		    m | child<1, 3> = r | child<1>;
		    m | child<2, 3> = r | child<2>;
		    m | child<3, 3> = r | child<3>;*/
			//data[i - 2] = mat_mul(data[i - 1], data[i]);
			//data[i - 2] = mat_mul_to(data[i - 1], data[i]);
		}
	}

    std::cout << (m | child<0, 0>) << ", " << (m | child<2, 2>) << '\n';
}

void rzx2_test()
{
    rzxmat2 m{};

	{
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
	}

	{
	    std::cout << "rzx2\n";
	    timer t{};
	    for (size_t i = 0; i < N; ++i)
		{
			//m = mat_mul(m, m);

			//[[msvc::flatten]]
		    m = +mat_mul(m, m);

			//m | child<0> = mat_mul_vec(m, m | component<3, 1>)/* | to<tuple>()*/;
			/*m | child<0, 3> = r | child<0>;
		    m | child<1, 3> = r | child<1>;
		    m | child<2, 3> = r | child<2>;
		    m | child<3, 3> = r | child<3>;*/
			//data[i - 2] = mat_mul(data[i - 1], data[i]);
			//data[i - 2] = mat_mul_to(data[i - 1], data[i]);
		}
	}

    std::cout << (m | child<0, 0>) << ", " << (m | child<2, 2>) << '\n';
}