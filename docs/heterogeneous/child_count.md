# rzx::`child_count`
在标头[<ruzhouxie/child.hpp>](../child.md)中定义
```cpp
template<class T>
inline constexpr size_t child_count;
```
表示`T`类型变量子级个数的常量。
- 当`T`是确定边界的数组类型或其引用时，`child_count<T>`为此数组的长度；  
- 否则，令`t`为表达式 [**`std::declval`**][declval]`<T>()`，`i`为一[`size_t`]()类型的整数，若表达式`t.get<i>(`[`custom_t`]()`{})`或`get<i>(t, custom_t{})`或`t.get<i>()`或`get<i>(t)`在`i`取`0`时良构，从`0`开始[^2]逐渐增加`i`的值，每次增加`1`，`child_count<T>`为满足以下条件时`i`的值；
  - 表达式`t.get<i>(custom_t{})`和`get<i>(t, custom_t{})`均满足: 非良构或者类型为`end_t`，`i >= std::tuple_size<std::remove_cvref_t<T>>::value`良构并且值为`false`；
  - 或者四个表达式全部都满足：非良构或者类型为`end_t`。
- 否则，若`std::tuple_size<std::remove_cvref_t<T>>::value`良构且值为0，`child_count<T>`是`0`。
- 否则，若`std::remove_cvref_t<T>`是一个聚合类，`child_count<T>`为此聚合类数据成员个数与`auto_supported_aggregate_max_size`中更小的一个值；
- 否则，`child_count<T>`是`0`。

[declval]:https://zh.cppreference.com/w/cpp/utility/declval "外部：https://zh.cppreference.com/w/cpp/utility/declval"