# 标头<ruzhouxie/tuple.hpp>
此头文件定义了聚合类的`ruzhouxie::tuple`，并提供了相关函数
## 包含
`<ruzhouxie/general.hpp>`
## 类
||||
|-|-|-|
| [**tuple**](tuple/tuple.md) | 实现固定大小的聚合类容器，它保有类型可以相异的元素 | `类模板` |
| [**std::tuple_size**\<ruzhouxie::tuple\>](tuple/tuple/tuple_size.md) | 在编译时获得 tuple 的大小 | `类模板特化`|
| [**std::tuple_element**\<ruzhouxie::tuple\>](tuple/tuple/tuple_element.md) | 获得指定元素的类型 | `类模板特化` |
## 函数
||||
| --- | --- | --- |
| [**make_tuple**](tuple/make_tuple.md) | 创建一个 tuple 对象，其类型根据各实参类型定义 | `函数模板` |
| [**fwd_as_tuple**](tuple/fwd_as_tuple.md) | 创建转发引用的 tuple | `函数模板` |
| [**get**(ruzhouxie::tuple)](tuple/tuple/get.md) | 元组式访问指定的元素 | `函数模板` |
| [**operator==**](tuple/operator_cmp.md) | 判断 tuple 中所有值是否都相等 | `函数模板` |
