# std::`tuple_size`\<ruzhouxie::tuple\>
在标头[<ruzhouxie/tuple.hpp>](../../tuple.md)中定义
```cpp
template< class... Types >
struct std::tuple_size< ruzhouxie::tuple<Types...> >
    : std::integral_constant<std::size_t, sizeof...(Types)> { };
```
提供对元组中元素数量的访问，作为编译时常量表达式。
## 辅助变量模板
```cpp
template< class T >
inline constexpr std::size_t tuple_size_v = tuple_size<T>::value;
```
---
---
## 继承自[std::integral_constant](https://zh.cppreference.com/w/cpp/types/integral_constant)
### 成员常量
||||
|-|-|-|
| **value**`[静态]` | sizeof...(Types) | `公开静态成员常量` |
### 成员函数
||||
|-|-|-|
| **operator std::size_t** | 将对象转换到[std::size_t](https://zh.cppreference.com/w/cpp/types/size_t)，返回`value` | `公开成员函数` |
| **operator()** | 返回`value` | `公开成员函数` |
## 成员类型
|类型|定义|
|-|-|
| value_type | [std::size_t](https://zh.cppreference.com/w/cpp/types/size_t) |
| type | [std::integral_constant](https://zh.cppreference.com/w/cpp/types/integral_constant)<[std::size_t](https://zh.cppreference.com/w/cpp/types/size_t), value>
 |
---
---
## 参阅
||||
|-|-|-|
| [结构化绑定](https://zh.cppreference.com/w/cpp/language/structured_binding) | 绑定指定的名字到初始化式的子对象或元组元素 |
| [**tuple_size**](https://zh.cppreference.com/w/cpp/utility/tuple_size) | 获得元组式类型的元素数 | `类模板` |
| [**std::tuple_size**\<std::tuple\>](https://zh.cppreference.com/w/cpp/utility/tuple/tuple_size) | 在编译时获得 tuple 的大小 | `类模板特化` |
| [**get**(ruzhouxie::tuple)](get.md) | 元组式访问指定的元素 | `函数模板` |