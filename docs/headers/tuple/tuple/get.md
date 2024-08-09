# `get`(ruzhouxie::tuple)
在标头[<ruzhouxie/tuple.hpp>](../../tuple.md)中定义
```cpp
//(1)
template< size_t I, class... Types >
std::tuple_element<I, ruzhouxie::tuple<Types...>>::type&
    get( ruzhouxie::tuple<Types...>& t ) noexcept;
```
```cpp
//(2)
template< size_t I >
std::tuple_element<I, ruzhouxie::tuple<Types...>>::type&&
    get( ruzhouxie::tuple<Types...>&& t ) noexcept;
```
```cpp
//(3)
template< size_t I >
const std::tuple_element<I, ruzhouxie::tuple<Types...>>::type&
    get( const ruzhouxie::tuple<Types...>& t ) noexcept;
```
```cpp
//(4)
template< size_t I >
const std::tuple_element<I, ruzhouxie::tuple<Types...>>::type&&
    get( const ruzhouxie::tuple<Types...>&& t ) noexcept;
```
1-4) 从元组提取第`I`个元素。`I`必须是\[ `​0`​, `sizeof...(Types)` \) 中的整数值。  
返回值类型中的`std::tuple_element<I, ruzhouxie::tuple<Types...>>::type`即是`Types...`中第`I`个类型。
## 参数
|||
|-|-|
|**t**|要提取内容的元组|
## 返回值
到`t`的被选中元素的引用。
## 参阅
||||
|-|-|-|
| [结构化绑定](https://zh.cppreference.com/w/cpp/language/structured_binding) | 绑定指定的名字到初始化式的子对象或元组元素 |