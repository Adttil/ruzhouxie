# rzx::`derived_from`
在标头[<ruzhouxie/general.hpp>](../headers/general.md "headers/general")中定义
```cpp
namespace rzx{
    template< class T >
    concept aggregated = std::is_aggregate_v<std::remove_cvref_t<T>>;
}
```
概念`rzx::aggregated<T>`当且仅当`std::remove_cvref_t<T>`为聚合类型才得到满足。
## 示例
```cpp

```