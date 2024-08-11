# rzx::`custom_t`
在标头[<ruzhouxie/general.hpp>](../headers/general.md "headers/general")中定义
```cpp
namespace rzx{
    struct custom_t{};
}
```
`rzx::custom_t`是空类类型，用于指示重载用于本库的定制点。在本库中的使用详见[`rzx::child_count`][child_count]、[`rzx::child`][child]。
## 注解
假设库中有一函数模板如下
```cpp
namespace rzx
{
    template<class T>
    constexpr auto y(T& t)
    {
        if constexpr(requires{ t.g(); })
        {
            return t.g();
        }
        else if constexpr(requires{ g(t); }) //adl查找
        {
            return g(t);
        }
        else
        {
            return 1; //默认的实现
        } 
    }
};
```
`rzx::y(t)`的结果依赖于`t.g()`或实参依赖查找的`g(t)`的结果，它是一个订制点，即它的行为对于不同类型的`t`可以自定义不同的行为。  
但是以上写法存在一个潜在的问题，假设某其他第三方库已经定义了如下类型:
```cpp
namespace lib
{
    struct X
    {
        int g(){ return 0; };
    };

    constexpr int g(X&){ return 0; }
}
```
且在此库中同名函数`g`的含义和作用与本库完全不同，它却影响了`rzx::y`的行为，但这不是我们想要的效果，并且不应侵入式的修改此第三方库的内容。  
为此需要使用`custom_t`做为标签类型来消歧义，将`rzx::y`修改如下：
```cpp
namespace rzx
{
    template<class T>
    constexpr auto y(T& t)
    {
        if constexpr(requires{ t.g(custom_t{}); })
        {
            return t.g(custom_t{});
        }
        else if constexpr(requires{ g(t, custom_t{}); }) //adl查找
        {
            return g(t, custom_t{});
        }
        else if constexpr(requires{ t.g(); })
        {
            return t.g();
        }
        else if constexpr(requires{ g(t); }) //adl查找
        {
            return g(t);
        }
        else
        {
            return 1; //默认的实现
        } 
    }
};
```
优先查找参数列表尾部多加了一个`rzx::custom_t`标签类型参数的版本，由于此类型是本库独特的，因此可以保证无歧义。  
此时可以通过给类型`lib::X`添加如下函数正确的为其提供`rzx::y`的定制：
```cpp
namespace lib
{
    constexpr int g(X&, ::rzx::custom_t)
    {
        return 233;
    }
}
```
现在对于类型`X`的对象`x`，调用`rzx::y(x)`的结果是`233`。

## 参阅
||||
|-|-|-|
| [**child_count**][child_count] | 给定类型的直接子级数量 | `常量模板` |
| [**child**][child] | 根据索引获取子级 | `适配器闭包对象` |

[child_count]:../heterogeneous/child_count.md
[child]:../heterogeneous/child.md