# ruzhouxie::tuple<Types...>::**get**
在标头[`<ruzhouxie/tuple.hpp>`](../../tuple.md)中定义
***
```cpp
//(1)
template< size_t I >
std::tuple_element<I, ruzhouxie::tuple<Types...>>::type&
    get()& noexcept;
```
```cpp
//(2)
template< size_t I >
std::tuple_element<I, ruzhouxie::tuple<Types...>>::type&&
    get()&& noexcept;
```
```cpp
//(3)
template< size_t I >
const std::tuple_element<I, ruzhouxie::tuple<Types...>>::type&
    get()const & noexcept;
```
```cpp
//(4)
template< size_t I >
const std::tuple_element<I, ruzhouxie::tuple<Types...>>::type&&
    get()const && noexcept;
```
1-4) 从元组提取第`I`个元素。`I`必须是\[ `​0`​, `sizeof...(Types)` \) 中的整数值。