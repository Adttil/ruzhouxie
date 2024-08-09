# ruzhouxie::**tuple**
在标头[`<ruzhouxie/tuple.hpp>`](../tuple.md)中定义
***
```cpp
template< class... Types >
struct tuple;
```
类模板`ruzhouxie::tuple`是固定大小的异质值的聚合。  它是一个聚合类。
## 模板形参
**Types...**    -  tuple 所存储的元素的类型。支持空列表。
## 数据成员
对于`Types...`中的每个`Ti`都有一个相应类型的数据成员，但是每个数据成员的名称不是公开接口的一部分，本库不保证这些名称是否会变化，因此访问它们需要使用函数模板[`get(ruzhouxie::tuple)`](tuple/get.md)。
## 非成员函数
||||
| --- | --- | --- |
| [`make_tuple`](make_tuple.md) | 创建一个 tuple 对象，其类型根据各实参类型定义 | `函数模板` |
| [`fwd_as_tuple`](fwd_as_tuple.md) | 创建转发引用的 tuple | `函数模板` |
| [`get(ruzhouxie::tuple)`](tuple/get.md) | 元组式访问指定的元素 | `函数模板` |
## 辅助类
||||
| --- | --- | --- |
| [`std::tuple_size<ruzhouxie::tuple>`](tuple/tuple_size.md) | 在编译时获得 tuple 的大小 | `类模板特化` |
| [`std::tuple_element<ruzhouxie::tuple>`](tuple/tuple_element.md) | 获得指定元素的类型 | `类模板特化` |
## [推导指引](tuple/deduction_guides.md)