#include <iostream>
#include <memory>
#include <bits/stdc++.h>

using namespace std;

int countTotal = 0;
class A
{
private:
    /* data */
public:
    int i;
    A(int n);
    ~A();
};

A::A(int n)
{
    i = n;
    ++countTotal;
}

A::~A()
{
    cout << i << " destructed" << endl;
    --countTotal;
}

int main() {
    vector<std::function<void()>> vecFunc;
    cout << "hello world" << endl;
    {
        // auto p = new A(2);
        A a(2);
        cout << countTotal << endl;
        cout << "int bracket" << endl;
        auto trans = std::move(a);
        auto print = [trans]() {
            cout << trans.i << endl;
        };
        cout << countTotal << endl;
        // 到这里没有析构
        // emplace_back caused a destruct?
        vecFunc.emplace_back(print);
        cout << countTotal << endl;
        // 在作用域中析构4次
    }
    cout << "out of bracket" << countTotal << endl; // 这里输出了-3 什么鬼
    // 按理说析构与构造一定一样多 应该是有点构造调用了拷贝构造 导致total没有+1
    cout << "end" << endl;
    // what's && here
    for (auto &&func : vecFunc)
    {
        func();
    }
    
    return 0;
}