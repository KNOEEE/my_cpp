#include <iostream>

template <int n> struct Fib
{
    enum { val = Fib<n - 1>::val + Fib<n - 2>::val };
};

template<> struct Fib<0> { enum { val = 0 }; };
template<> struct Fib<1> { enum { val = 1 }; };

int main()
{
    std::cout << Fib<0>::val << std::endl;
    std::cout << Fib<1>::val << std::endl;
    std::cout << Fib<2>::val << std::endl;
    std::cout << Fib<3>::val << std::endl;
    std::cout << Fib<4>::val << std::endl;
    std::cout << Fib<5>::val << std::endl;
    std::cout << Fib<10>::val << std::endl;
    getchar();
    return 0;
}