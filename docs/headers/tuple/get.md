# **get**(ruzhouxie::tuple)
在标头[`<ruzhouxie/tuple.hpp>`](../../tuple.md)中定义
***
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