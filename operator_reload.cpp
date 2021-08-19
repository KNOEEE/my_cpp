#include <iostream>
#include <memory>

using namespace std;

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
};

int main() {
  Square s1;
  s1.a = 10;
  s1.b = 3;
  cout << s1.get_perimeter() << endl;
  Square* s2 = new Square(1, 4);
  cout << s2->get_perimeter() << endl;
  shared_ptr<Square> s3;
  s3.reset(new Square(4, 5));
  cout << s3->get_perimeter() << endl;
}
