#include <iostream>

struct X{
    int&& v;
    decltype(auto) foo(){ return (v); }
};

template<typename T>
struct Y{
    T v;
    auto&& foo() { return (v); }
};

template<typename T>
struct Z{
    T&& v;
    decltype(auto) foo() { return (v); }
};

int main()
{
    int i = 0;
    //X x{ std::move(i) }; //ok
    Y<int&&> x{ std::move(i) }; //error
    //Z<int> x{ std::move(i) }; //ok
    //Z<int&&> x{ std::move(i) }; //ok
    
    std::cout << x.foo() << "\n";
    return 0;
}