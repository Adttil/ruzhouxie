# `operator==`(ruzhouxie::tuple)
在标头[<ruzhouxie/tuple.hpp>](../../headers/tuple.md)定义
```cpp
template< class... TTypes, class... UTypes >
constexpr bool operator==( const ruzhouxie::tuple<TTypes...>& lhs,
                           const ruzhouxie::tuple<UTypes...>& rhs ) = default;
```
以`operator==`比较元组`lhs`的每个元素和元组`rhs`的对应元素。
在每对对应元素都相等时返回`true`。
## 参数
|||
|-|-|
|**lhs, rhs**|要比较的元组|
## 返回值
在对于所有 [ `0`​, `sizeof...(Types)` ) 中的`i`都满足 [`get`](get.md)`<i>(lhs) == `[`get`](get.md)`<i>(rhs)`时返回`true`，否则返回`false`。对两个空元组返回`true`。