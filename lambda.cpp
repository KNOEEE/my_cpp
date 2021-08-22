#include<iostream>
#include<functional> // std::function
using namespace std;
      
typedef enum
{     
    add = 0,
    sub,
    mul,
    divi
}type;
      
int main()
{     
  // doc
  // https://www.cnblogs.com/DswCnblog/p/5629165.html
  // https://en.cppreference.com/w/cpp/language/lambda
    int a = 10;
    int b = 20;
      
    auto func = [=](type i)->int {
        switch (i) {
            case add:
                return a + b;
            case sub:
                return a - b;
            case mul:
                return a * b;
            case divi:
                return a / b;
        }
    };
      
    cout << func(add) << endl;
    // type of lambda
    function<int(int, int)> f = [](int a, int b) {return a + b;};
    cout << f(a, b * 2) << endl;
}