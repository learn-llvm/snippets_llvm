#include "Shape.hh"
#include <vector>
#include <iostream>

using namespace llvm;

int main(void) {
  std::vector<Shape *> shapes;
  Square *square = new Square(2);
  Circle *circle = new Circle(1);
  shapes.push_back(square);
  shapes.push_back(circle);
  for (Shape *ele : shapes) {
    if (ele == nullptr) continue;
    if (Square *s = dyn_cast<Square>(ele)) {
      std::cout << *s << std::endl;
    } else if (Circle *c = dyn_cast<Circle>(ele)) {
      std::cout << *c << std::endl;
    }
  }
  for (auto ele : shapes) {
    delete ele;
  }
  return 0;
}
