# ruzhouxie::`child`
在标头[<ruzhouxie/child.hpp>](../child.md)中定义
```cpp
template<indexical auto...Indexes>
inline constexpr /*未指定*/ child = /*未指定*/;
```
#### 调用签名
```cpp
template<indexical auto...Indexes, typename T>
constexpr /*见下文*/ child(T&& t);
```
用于获取t的指定分量的适配器对象。  
- 对于`child<>(t)`时，直接返回`t`(完美转发)；  
- 对于`child<I>(t)`，`I`是一个整数时
  - 若`t`是已知边界的数组，返回`t`的第`I`个元素
  - 若