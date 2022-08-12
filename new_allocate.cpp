#include <iostream>

using namespace std;

struct A {
  int data[100];
  A() { cout << "constructed [" << this << "]\n"; }
  ~A() { cout << "destructed\n"; }
};

int main() {
  A* p1 = new A();
  A* p2 = new (p1) A();
  // Global: All three versions of operator new are declared in the 
  // global namespace, not within the std namespace.
  A* p3 = (A*) ::operator new (sizeof(A));
  delete p1;
  delete p3;
}