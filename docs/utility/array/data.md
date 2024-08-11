# rzx::array<T, N>::`data`
在标头[<ruzhouxie/array.hpp>](../../headers/array.md "headers/array")中定义
```cpp
constexpr T* data() noexcept;
constexpr const T* data() const noexcept;
```
返回指向作为元素存储工作的底层数组的指针。返回的指针使得范围 [ `data()`, `data() + size()` ) 始终是有效范围，即使容器为空（此时`data()`不可解引用）。
## 返回值
指向底层元素存储的指针。对于非空容器，返回的指针与首元素地址比较相等。
## 注解
如果[`size()`](size.md)是`​0`​，那么`data()`有可能会也有可能不会返回空指针。
## 参见
||||
|-|-|-|
|[**size**](size.md)|返回元素数|`公开成员函数`|

## 外部链接
||||
|-|-|-|
|[std::array::**data**](https://zh.cppreference.com/w/cpp/container/array/data)|直接访问底层连续存储|`公开成员函数`|