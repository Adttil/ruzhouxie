# ruzhouxie::`make_tuple`
在标头[<ruzhouxie/tuple.hpp>](../../headers/tuple.md)定义
```cpp
template< class... Types >
constexpr ruzhouxie::tuple<VTypes...> make_tuple( Types&&... args );
```
创建元组对象，从实参类型推导目标类型。  
对于`Types...`中的每个`Ti`，`Vtypes...`中的对应类型`Vi`为`std::decay<Ti>::type`。
## 参数
|||
|-|-|
| args... | 为之构造元组的零或更多实参 |
## 返回值
含给定值的`ruzhouxie::tuple`对象，如同用`ruzhouxie::tuple<VTypes...>(std::forward<Types>(t)...)`创建。
## 参阅
||||
| --- | --- | --- |
| [**fwd_as_tuple**](fwd_as_tuple.md) | 创建转发引用的 tuple | `函数模板` |