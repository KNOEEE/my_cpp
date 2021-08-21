#include <iostream>
#include <memory>

using namespace std;

// TODO 
// heuristic search, a* algorithm
// https://github.com/microsoft/vscode/issues/113458
// s**t vscode
class Square
{
public:
  Square(/* args */) : a(0), b(0) {}
  explicit Square(int p_a, int p_b)
      : a(p_a), b(p_b) {}
  int a, b;
  int get_perimeter() {
    return a * b;
  }

  // try reload operator (class member)
  Square operator+(const Square& that) {
    Square s;
    // `this` can be omitted
    s.a = a + that.a;
    s.b = this->b + that.b;
    return s;
  }

  Square operator-(int val) {
    Square s;
    s.a = a - val;
    s.b = b - val;
    return s;
  }
};

Square operator^(const Square& aa, const Square& bb) {
  int a = aa.a + bb.a;
  int b = aa.b * bb.b;
  return Square(a, b);
}

int main() {
  Square s1;
  s1.a = 10;
  s1.b = 3;
  cout << s1.get_perimeter() << endl;
  Square* s2 = new Square(1, 4);
  cout << s2->get_perimeter() << endl;
  shared_ptr<Square> s3, s4;
  s3.reset(new Square(4, 5));
  cout << s3->get_perimeter() << endl;
  // notice arguments for make_shared
  s4 = make_shared<Square>(2, 4);
  cout << s4->get_perimeter() << endl;
  Square s5 = s1 + *s4;
  cout << s5.get_perimeter() << endl;
  cout << (s5 - 2).get_perimeter() << endl;
  auto s6 = s1 ^ (s5 - 4);
  cout << s6.a << " " << s6.b <<
      " " << s6.get_perimeter() << endl;
}
