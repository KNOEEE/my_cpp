#include <iostream>
#include <memory>

using namespace std;

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
}

A::~A()
{
    cout << i << " destructed" << endl;
}

int main() {
    shared_ptr<A> sp1(new A(2));
    // A(2)同时由sp1和sp2保管
    shared_ptr<A> sp2(sp1);
    shared_ptr<A> sp3;
    sp3 = sp2;
    cout << sp1->i << ", " << sp2->i << ", " << sp3->i << endl;
    // get返回托管的指针
    A* p = sp3.get();
    // reset导致托管新的指针 此时sp1托管A(3)
    sp1.reset(new A(3));
    sp2.reset(new A(4));
    cout << sp1->i << endl;
    sp3.reset(new A(5));
    {
      cout << "bracket start\n"; 
        auto p = new A(2);
        A a(2);
        cout << "int bracket" << endl;
        // 对象生命周期结束
        // normal pointer must be deleted manually
        delete p;
    }
    cout << "end" << endl;
    return 0;
}