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
对于`Types...`中的每个`Ti`都有一个相应类型的数据成员，但是每个数据成员的名称不是公开接口的一部分，本库不保证这些名称是否会变化，因此访问它们需要使用函数模板[`get`](get.md)。

## 非成员函数
[`get`](get.md)