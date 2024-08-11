# ruzhouxie::tuple 的推导指引
在标头[<ruzhouxie/tuple.hpp>](../../headers/tuple.md)中定义
```cpp
template<typename...T>
tuple(T...) -> tuple<std::decay_t<T>...>;
```
为[`rzx::tuple`](guid.md)提供这些推导指引，以涵盖隐式推导指引缺失的边界情况，特别是不可复制的实参和数组到指针转换。