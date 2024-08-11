# rzx::`array`
在标头[<ruzhouxie/general.hpp>](../headers/array.md "headers/array")中定义
```cpp
namespace rzx{
#if __STDC_HOSTED__
    template< typename T, size_t N >
    class array;
#else
#include <array>
    using std::array;
#endif
}
```
若C++标准非独立实现，`rzx::array`就是[**`std::array`**][std::array]，否则它是一个类似`std::array`但更加精简的原生数组包装类。此页面以下的说明表示的是`rzx::array`至少需要拥有的功能。
## 模板形参
|||
|-|-|
| T | 元素类型。必须为可移动构造和可移动赋值 |
| N | 数组中的元素数量或`0`​ |
## 成员类型
| 成员类型 | 定义 |
|-|-|
| value_type | T |
## 成员函数
### 隐式定义的成员函数
||||
|-|-|-|
| **(构造函数)**`(隐式声明)` | 遵循聚合初始化的规则初始化 array（注意默认初始化可以导致非类的`T`的不确定值） | `公开成员函数` |
|**(析构函数)**`(隐式声明)`|销毁 array 的每个元素|`公开成员函数`|
|**operator=**`(隐式声明)`|以来自另一 array 的每个元素重写 array 的对应元素|`公开成员函数`|
### 其他
||||
|-|-|-|
|[**operator[]**]()|访问指定的元素|`公开成员函数`|
|[**data**]()|直接访问底层连续存储|`公开成员函数`|
|[**begin**]()|返回指向起始的迭代器|`公开成员函数`|
|[**end**]()|返回指向末尾的迭代器|`公开成员函数`|
|[**size**]()|返回元素数|`公开成员函数`|
## 非成员函数
||||
|-|-|-|
|[**operator==**]()|两个`array`的每一个对元素是否都相等|`函数模板`|
|[**get**(rzx::array)]()|两个`array`的每一个对元素是否都相等|`函数模板`|
## 辅助类
||||
|-|-|-|
|[**std::tuple_size**\<rzx::array\>]()|获得 array 的大小|`类模板特化`|
|[**std::tuple_element**\<rzx::array\>]()|获得 array 元素的类型|`类模板特化`|
## [推导指引]()
## 外部链接
||||
|-|-|-|
|[**std::array**][std::array]|固定大小的原位连续数组|`类模板`|

[std::array]:https://zh.cppreference.com/w/cpp/container/array

  

 



  

 

