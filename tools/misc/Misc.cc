#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/Hashing.h"
#include "llvm/ADT/PackedVector.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"

#include <algorithm>
#include <iostream>

#include "Person.hh"
#include "Shape.hh"

using namespace llvm;

/// {
int test_yaml() {
  Person p1("hello", 1);
  Person p2("world");
  std::vector<Person> persons;
  persons.push_back(p1);
  persons.push_back(p2);
  yaml::Output yout(llvm::errs());
  yout << persons;
  return 0;
}
/// }

template <typename Callable>
void takeCallBack(Callable callback, bool verbose) {
  callback(verbose);
}

enum State { None = 0x0, first = 0x1, second = 0x2, both = 0x3 };

void test_PackedVector() {
  PackedVector<State, 2> vec1;
  PackedVector<State, 2> vec2;
  vec1.push_back(State::first);
  vec2.push_back(State::second);
  vec1 |= vec2;
}

int test_rtti(void) {
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

void test_StringRef() {
  StringRef ref("hello");
  errs() << ref << "\n";
  hash_code code = hash_value(ref);
  errs() << hash_value(code) << '\n';
  StringRef lh, rh;
  std::tie(lh, rh) = ref.split('l');
  errs() << lh << '\t' << rh << '\n';
  std::tie(lh, rh) = ref.split(StringRef("ll"));
  errs() << lh << '\t' << rh << '\n';
}

void test_Twine(bool verbose) {
  errs() << "verbose: " << verbose << "\n";
  StringRef s1 = "hello world";
  std::string s2 = "goodbye";
  Twine t0;
  Twine t1(s1);
  Twine t2(s2);
  char c = 'c';
  unsigned i = 12345;
  Twine t_c(c);
  Twine t_i(i);
  Twine t = t1 + t_c;
  Twine t_t = t + t2;
#define X printRepr
#define dumpWithNewLine(twine) \
  errs() << #twine << '\t';    \
  twine.X(errs());             \
  errs() << '\n';
  dumpWithNewLine(t0);
  dumpWithNewLine(t1);
  dumpWithNewLine(t2);
  dumpWithNewLine(t_c);
  dumpWithNewLine(t_i);
  dumpWithNewLine(t);
  dumpWithNewLine(t_t);
#undef dumpWithNewLine
  StringRef ref = t_t.getSingleStringRef();
  errs() << ref << '\n';
}

void test_ArrayRef() {
  std::vector<int> v{1, 2, 3};
  int a[4] = {1, 2, 3, 4};
  ArrayRef<int> ar1(a);
  errs() << ar1.size() << '\n';
  ArrayRef<int> ar2(v);
  errs() << ar2.size() << '\n';
  ArrayRef<int> ar3{1, 2, 3, 4};
  errs() << ar3.size() << '\n';
}

void test_StringSwitch() {
  std::vector<StringRef> strs{"hello",  "World", "nihao", "!",
                              "jiejie", "how",   "none"};
  for (auto str : strs) {
    int si = StringSwitch<int>(str)
                 .Case("hello", 1)
                 .Case("World", 2)
                 .Cases("nihao", "shijie", "!", 3)
                 .EndsWith("jie", 4)
                 .StartsWith("h", 5)
                 .Default(0);
    errs() << format("%-10s %d\n", str.data(), si);
  }
}

int main() {
  test_StringRef();
  //  takeCallBack(test_Twine, true);
  test_ArrayRef();
  test_StringSwitch();
  test_PackedVector();
  test_rtti();
  test_yaml();
  return 0;
}
