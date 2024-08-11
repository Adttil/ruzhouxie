# rzx::`derived_from`
在标头[<ruzhouxie/general.hpp>](../headers/general.md "headers/general")中定义
```cpp
template< class Derived, class Base >
concept derived_from = std::derived_from<std::remove_cvref_t<Derived>, std::remove_cvref_t<Base>>;
```
如上定义，概念`rzx::derived_from`就是[**`std::derived_from`**][std::derived_from]的对参数忽略 cv 限定和去除引用的版本。
## 示例
```cpp
#include <ruzhouxie/general.hpp>
 
class A {};
 
class B : public A {};
 
class C : private A {};
 
// std::derived_from == true 仅对公有继承或完全相同的类（非基础类型）成立
static_assert(std::derived_from<B, B> == true);        // 相同的类: true
static_assert(std::derived_from<int, int> == false);   // 相同的基础类型: false
static_assert(std::derived_from<C, A> == false);       // 私有继承: false 
static_assert(std::derived_from<B, A> == true);        // 公有继承: true
static_assert(std::derived_from<B&&, A&> == true);     // 添加引用不影响结果
static_assert(std::derived_from<const B, A&&> == true);// 添加cv限定不影响结果

int main() {}
```
## 外部链接
||||
|-|-|-|
|[**std::derived_from**][std::derived_from]| 指定一个类型派生自另一类型 | `概念` |

[std::derived_from]:https://zh.cppreference.com/w/cpp/concepts/derived_from