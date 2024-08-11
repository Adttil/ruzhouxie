# rzx::`choice_t`
在标头[<ruzhouxie/general.hpp>](../headers/general.md "headers/general")中定义
```cpp
template <class Strategy = bool>
struct choice_t{
    Strategy strategy{};
    bool nothrow = false;
};
```
`choice_t`是一个如上定义的简单聚合类模板，它是用于指示函数静态分支的辅助类型。
## 模板参数
|||
|-|-|
| **Strategy** | 策略的类型，通常是一个枚举类型或者布尔值 |
## 数据成员
| 成员名 | 类型 | 说明 |
|-|-|-|
| strategy | Strategy | 指示函数应该走哪一个静态分支 |
| nothrow | bool | 指示函数走该静态分支时是否不会抛出 |
## 示例
```cpp
#include<iostream>
#include<ruzhouxie/general.hpp>

class Dog{
    void run()//狗会跑
    {
        std::cout << "The dog ran." << std::endl;
    }
};

class Bird{
    void fly()//鸟会飞
    {
        std::cout << "The bird flew." << std::endl;
    }
};

//移动策略
enum class move_strategy_t{
    none,
    run,
    fly
};

//选择移动方式，并且获知此种方式下是否不会抛出
template<typename T>
consteval rzx::choice_t<move_strategy_t> move_choose()
{
    if constexpr(requires{ std::declval<T>().fly(); })//能飞就飞
    {
        return { move_strategy_t::fly, noexcept(std::declval<T>().fly()) };
    }
    else if constexpr(requires{ std::declval<T>().run(); })//否则能跑就跑
    {
        return { move_strategy_t::run, noexcept(std::declval<T>().run()) };
    }
    else
    {
        return { move_strategy_t::none };
    }
}

//移动
template<typename T>
    requires (move_choose<T>().strategy != move_strategy_t::none)
constexpr void move(T&& t) noexcept(move_choose<T>().nothrow)
{
    move_strategy_t strategy = move_choose<T>().strategy;
    if constexpr(strategy == move_strategy_t::fly)
    {
        std::forward<T>(t).fly();
    }
    else if constexpr(strategy == move_strategy_t::run)
    {
        std::forward<T>(t).run();
    }
}

int main()
{
    Dog dog{};
    Bird bird{};
    move(dog);
    move(bird);
}
```
输出:
> The dog ran.  
> The bird flew.