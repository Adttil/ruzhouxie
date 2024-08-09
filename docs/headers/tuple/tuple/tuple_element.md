# std::`tuple_element`\<ruzhouxie::tuple>
在标头[<ruzhouxie/tuple.hpp>](../../tuple.md)中定义
```cpp
template< std::size_t I, class... Types >
struct std::tuple_element< I, ruzhouxie::tuple<Types...> >;
```
提供对元组元素类型的编译时索引访问。
## 成员类型
|成员类型|定义|
|-|-|
|type|元组的第`I`元素的类型，其中`I`在[ `​0`​, `sizeof...(Types)` ) 中
|
## 参阅
||||
|-|-|-|
| [结构化绑定](https://zh.cppreference.com/w/cpp/language/structured_binding) | 绑定指定的名字到初始化式的子对象或元组元素 |
| [**tuple_element**](https://zh.cppreference.com/w/cpp/utility/tuple_element) | 获得元组式类型的元素类型 | `类模板` |