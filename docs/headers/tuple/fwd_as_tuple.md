# ruzhouxie::**fwd_as_tuple**
在标头[`<ruzhouxie/tuple.hpp>`]()定义
```cpp
template< class... Types >
constexpr ruzhouxie::tuple<Types&&...> fwd_as_tuple( Types&&... args );
```
构造到`args`中的各实参的引用的元组，适于作为实参转发给函数。该元组在以右值为实参时拥有右值引用数据成员，否则拥有左值引用数据成员。
## 参数
|||
|-|-|
| args... | 为之构造元组的零或更多实参 |
## 返回值
如同以`ruzhouxie::tuple<Types&&...>(std::forward<Types>(args)...)`创建的`ruzhouxie::tuple`对象。
## 参阅
||||
| --- | --- | --- |
| [`make_tuple`](make_tuple.md) | 创建一个 tuple 对象，其类型根据各实参类型定义 | `函数模板` |