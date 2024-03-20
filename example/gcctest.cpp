#include <iostream>

struct end_t
{
    virtual void foo() = 0;
};

end_t end();

template<size_t N>
struct X 
{
    template<size_t I>
    auto get()
    {
        if constexpr(I < N)
        {
            return I;
        }
        else
        {
            return end();
        }
    }
};

struct Y{};

int main()
{
    X<0uz> x{};
    Y y{};

    return 0;
}