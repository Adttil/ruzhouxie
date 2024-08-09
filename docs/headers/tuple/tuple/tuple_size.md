# std::**tuple_size**\<std::tuple\>
在标头[`<ruzhouxie/tuple.hpp>`](../../tuple.md)中定义
```cpp
template< class... Types >
struct tuple_size< ruzhouxie::tuple<Types...> >
    : std::integral_constant<std::size_t, sizeof...(Types)> { };
```
提供对元组中元素数量的访问，作为编译时常量表达式。
## 辅助变量模板
```cpp
template< class T >
inline constexpr std::size_t tuple_size_v = tuple_size<T>::value;
```
