#ifndef SHAPE_HPP
#define SHAPE_HPP

#include "llvm/Support/Casting.h"
#include <ostream>
#include "Common.hh"

class Shape {
 public:
  enum ShapeKind {
    SK_Square,
    SK_Circle
  };

 private:
  DISALLOW_COPY_AND_ASSIGN(Shape);
  ShapeKind const kind_;

 public:
  Shape(ShapeKind kind) : kind_(kind) {}
  ShapeKind getKind() const { return kind_; }
  virtual double computeArea() const = 0;
  virtual ~Shape() {}
};

class Square : public Shape {
 private:
  double sideLength_;

 public:
  Square(double sideLength) : Shape(SK_Square), sideLength_(sideLength) {}
  double computeArea() const override { return sideLength_ * sideLength_; }
  static bool classof(Shape const *S) { return S->getKind() == SK_Square; }
  friend std::ostream &operator<<(std::ostream &os, Square const &square) {
    os << "square, sideLength_=" << square.sideLength_
       << ", area=" << square.computeArea();
    return os;
  }
  ~Square() {}
};

class Circle : public Shape {
 private:
  double const PI = 3.1415926;
  double radius_;

 public:
  Circle(double radius) : Shape(SK_Circle), radius_(radius) {}
  double computeArea() const override { return PI * radius_ * radius_; }
  static bool classof(Shape const *S) { return S->getKind() == SK_Circle; }
  friend std::ostream &operator<<(std::ostream &os, Circle const &circle) {
    os << "circle, radius_=" << circle.radius_
       << ", area=" << circle.computeArea();
    return os;
  }
  ~Circle() {}
};

#endif
