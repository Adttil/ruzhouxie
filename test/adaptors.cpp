#include "test_tool.h"
#include <ruzhouxie\adaptors.h>
#include <ruzhouxie\result.h>
//#include <ruzhouxie\constant.h>

using namespace ruzhouxie;

struct X { int a; double b; };
struct Y { double a; double b; };

int main()
{
	//MAGIC_CHECK(branched<tuple<int, float>>, true);
	//auto&& [a, b] = lazy_zip_transform(std::plus<>{}, X{ 1, 3.04 }, std::array{ 232.0, 0.1 }) | to<tuple>();
	auto r = zip_transform(std::plus<>{}, X{ 1, 3.04 }, std::array{ 232.0, 0.1 }) | make_tree<std::tuple<double, double>>;
	auto&& [a, b] = r;
	MAGIC_CHECK(a, 233.0);
	MAGIC_CHECK(b, 3.14);

	auto t = zip_transform(std::plus<>{}, X{ 1, 3.04 }, std::array{ 232.0, 0.1 });
	MAGIC_SHOW_TYPE(t);
	MAGIC_SHOW_TYPE(t | child<0>);
	MAGIC_SHOW_TYPE(t | child<1>);
}