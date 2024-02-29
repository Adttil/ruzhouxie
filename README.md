![cover](docs/assets/cover.jpg)
`RuZhouXie`是一个通配的异构几何库，同时也是一个`std::ranges`风格的异构运算库。它基于标准`c++23`并且是 `header-only`、无依赖的。它使您可以使用同一套向量、矩阵相关函数操作储存格式各不相同的具体几何类型，同时使您可以像用`std::ranges`库一样方便的操作`tuple`等异构容器。

- [RuZhouXie](#ruzhouxie)
  - [基本几何库](#基本几何库)
  - [异构的“范围库”](#异构的范围库)
  - [通配的异构几何库](#通配的异构几何库)
  - [编译期表达式重整优化](#编译期表达式重整优化)
  - [第三方库](#第三方库)

## 基本几何库
就如同`glm`库一样，提供了一套默认的几何类型和各种几何操作。
```cpp
namespace rzx = ruzhouxie;

rzx::mat<2, 2> matrix{
    rzx::vec<2>{ 1.0, 2.0 },
    rzx::vec<2>{ 3.0, 4.0 }
};
rzx::vec<2> vector{ 1.0, 2.0 };

rzx::vec<2> result = +rzx::mat_mul_vec(matrix, vector);
```
由于本库的函数大多为惰性运算，前面写+可隐式转换为任意结构相匹配的类型，这里等价于：
```cpp
rzx::vec<2, double> result = rzx::mat_mul_vec(matrix, vector) | rzx::to<ruzhouxie::vec<2>>();
```
## 异构的“范围库”
就如c++20标库中新增的范围库一样，你可以用类似的风格方便的进行异构运算，这使你可以免除一些重复性代码。
```cpp
namespace rzx = ruzhouxie;

struct Y{ std::string str; int a; float b; };
Y y{ "that's", 133, 114000.0f };
auto tpl = std::tuple{ " pretty good!", 200.0f, 514 } };

//{ "that's pretty good", 233.0f, 114514.0f }
auto result = rzx::zip_transform(std::plus<void>{}, y, tpl) | rzx::to<rzx::tuple>();
```

## 通配的异构几何库
实际上你可以将任意类型视为你的几何类型来进行操作。
如果它是一个`tuple-like`的类型、聚合类或定长数组，那么不用做任何事就可以被统一处理：
```cpp
namespace rzx = ruzhouxie;

struct special_vec3{ float x, float y, int h };

special_vec3 vector1{ 3.0f, 4.0f, 0 };
std::tuple vector2{ 1.0, 2, 3.0f };

rzx::tuple<double, double, double> result1 = +rzx::add(vector1, vector2);
//如果要使用库定义的运算符重载，则至少有一个操作数要套上rzx::view
auto result2 = rzx::view{ vector1 } - vector2 | rzx::to<rzx::tuple>();

std::tuple matrix{ 
    std::array{ 1, 0, 0 },
    std::tuple{ 0, 1, 0 },
    rzx::tuple{ 0, 0, 1 }
 };
 std::array<double, 3> = +rzx::mat_mul_vec(matrix, special_vec3);
```
否则，你可以通过tag-invoke的方式定制行为：
```cpp
namespace rzx = ruzhouxie;

template<size_t count>
struct iota_t{ 
    size_t begin;

    template<size_t I>
    friend constexpr decltype(auto) tag_invoke(rzx::child<I>, const iota_t& self)noexcept
    {
        if constexpr(I >= count)
        {
            return;//这能避免foo_t被看作具有一个分量的聚合类
        }
        else
        {
            return begin + I;
        }
    }
};

iota_t<2> vec1{ 1 };
iota_t<2> vec2{ 3 };
size_t result = rzx::dot(vec1, vec2);//{ 1, 2 } 点乘 { 3, 4 } = 11
```
由于可以使用异构的几何类型，可以轻易做到一些神奇的事情，比如部分元素为编译期常量的矩阵：3D程序中经常使用的齐次矩阵，在进行投影变换前，其最后一行始终为{ 0, 0, 0, 1 }, 通过本库我们可以不实际储存这行的值，但是用一样方式来操作：
```cpp
namespace rzx = ruzhouxie;

using special_mat = rzx::tuple<
    rzx::vec<4>,
    rzx::vec<4>,
    rzx::vec<4>,
    rzx::tuple<rzx::constant_t<0>, rzx::constant_t<0>, rzx::constant_t<0>, rzx::constant_t<1>>
>;
//表示位移(3, 4, 0)的齐次矩阵
special_mat shift340{ 
    { 1.0, 0.0, 0.0, 3.0 },
    { 0.0, 1.0, 0.0, 4.0 },
    { 0.0, 0.0, 1.0, 0.0 },
    {}
 };
//表示位移(6, 8, 0)的齐次矩阵
special_mat shift680 = +rzx::mat_mul(shift340, shift340);
```
`rzx::constant_t`是一个表示常量的空类，类似于std::integral_constant，但是增加了一些运算相关的重载，比如使得`rzx::constant_t<0>{} * [能与int相乘的类型的任意对象]`的结果始终为`rzx::constant_t<0>{}`。通过这种方法使得矩阵乘法的运算量大大减少，在clang18上编译测试，最后一行采用编译期常量的齐次矩阵乘法比常规方式快62%。

## 编译期表达式重整优化
库中的惰性表达式再最终执行前会进行重组，以提高效率，这包括：对象在最后一次使用时进行完美转发、去除重复运算、去除无用运算等等。

例如下面的代码从一个`tuple<trace, trace>{ e0, e1 }`类型对象的右值引用重布局并构造一个`tuple<trace, trace, trace, trace, trace>{ e0, e1, e0, e1, e1 }`, 两个分量均在最后一次被使用时被移动。

```cpp
namespace rzx = ruzhouxie;

struct trace
{
    trace() { std::puts("trace();"); };
    trace(const trace&) { std::puts("trace(const trace&);"); }
    trace(trace&&) { std::puts("trace(trace&&);"); }
    trace& operator=(const trace&) { std::puts("trace& operator=(const trace&);"); return *this; }
    trace& operator=(trace&&) { std::puts("trace& operator=(trace&&);"); return *this; }
    ~trace() { std::puts("~trace();"); }
};

int main()
{
    constexpr auto layout = std::array//把[e0, e1]看作[e0, e1, e0, e1, e1]的布局
    {
        std::array{0uz}, std::array{1uz}, std::array{0uz}, std::array{1uz}, std::array{1uz}
    };

    std::array vector{ trace{}, trace{} };

    std::puts("==================");
    rzx::vec<5, trace> result = +(std::move(vector) | rzx::as_ref | rzx::relayout<layout>);
    std::puts("==================");
}
```

其中`rzx::as_ref`的作用是将右值引用仍然按引用储存，默认情况下对于右值引用都是按值存储以防止悬垂。
这里两个Tr对象都在最后一次使用时进行了完美转发，从而减少了复制构造的次数。程序输出如下：
```
trace();
==================
trace(const trace&);
trace(const trace&);
trace(trace&&);
trace(const trace&);
trace(trace&&);
==================
~trace();
~trace();
~trace();
~trace();
~trace();
~trace();
~trace();
```

## 第三方库
test文件夹下的测试使用了magic-cpp库来进行类型可视化：https://github.com/16bit-ykiko/magic-cpp