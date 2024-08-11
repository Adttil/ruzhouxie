# rzx::array<T, N>::`operator[]`
在标头[<ruzhouxie/array.hpp>](../../headers/array.md "headers/array")中定义
```cpp
T& operator[]( size_t pos );
const T& operator[]( size_t pos ) const;
```
返回位于指定位置`pos`的元素的引用。不进行边界检查。
## 参数
|||
|-|-|
|**pos**|要返回的元素的位置|
## 返回值
到所需元素的引用。