#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/Hashing.h"
#include "llvm/ADT/PackedVector.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/MD5.h"
#include "llvm/Support/MemoryBuffer.h"

#include <algorithm>
#include <iostream>

#include "Person.hh"
#include "Shape.hh"

using namespace llvm;

// https://modocache.io/llvm-memory-buffer

/// {
int test_yaml() {
  errs() << "\n\n===" << __PRETTY_FUNCTION__ << "\n";
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


void test_format() {
  errs() << "\n\n===" << __PRETTY_FUNCTION__ << "\n";
  Person p("Hongxu", 18);
  errs() << formatv("{0}", p) << "\n";

//  errs() << formatv("{0} ({1:P})", 7, 0.35) << "\n";
  errs() << formatv("{0,=7}\n", 'a');
  errs() << formatv("{0,-7}\n", 'a');
  errs() << formatv("{0,+7}\n", 'a');

//  std::vector<int> V = {8, 9, 10};
//  errs() << formatv("{0}", make_range(V.begin(), V.end())) << "\n";
//  errs() << formatv("{0:$[+]}", make_range(V.begin(), V.end())) << "\n";
//  errs() << formatv("{0:$[ + ]@[x]}", make_range(V.begin(), V.end())) << "\n";
}

void test_md5() {
  errs() << "\n\n===" << __PRETTY_FUNCTION__ << "\n";
  auto fname = "/bin/ls";
  auto res = MemoryBuffer::getFile(fname);
  if (!res) {
    errs() << res.getError().message() << "\n";
  } else {
    auto &buf = res.get();
    auto s = buf->getBuffer();
    SmallString<32> ss;
    MD5 md5;
    md5.update(s);
    MD5::MD5Result md5Result;
    md5.final(md5Result);
    errs() << fname << " MD5: " << md5Result.digest() << "\n";
  }

}

template<typename Callable>
void takeCallBack(Callable callback, bool verbose) {
  callback(verbose);
}

enum State { None = 0x0, first = 0x1, second = 0x2, both = 0x3 };

void test_PackedVector() {
  errs() << "\n\n===" << __PRETTY_FUNCTION__ << "\n";
  PackedVector<State, 2> vec1;
  PackedVector<State, 2> vec2;
  vec1.push_back(State::first);
  vec2.push_back(State::second);
  vec1 |= vec2;
}

int test_rtti(void) {
  errs() << "\n\n===" << __PRETTY_FUNCTION__ << "\n";
  std::vector<Shape *> shapes;
  Square *square = new Square(2);
  Circle *circle = new Circle(4);
  shapes.push_back(square);
  shapes.push_back(circle);
  for (Shape *ele : shapes) {
    if (ele == nullptr) {
      continue;
    }
    errs() << formatv("SHAPE: {}\n", ele);
    if (Square *s = dyn_cast<Square>(ele)) {
      errs() << formatv("{0}\n", *s);
    } else if (Circle *c = dyn_cast<Circle>(ele)) {
      errs() << formatv("{0}\n", *c);
    }
  }
  for (auto ele : shapes) {
    delete ele;
  }
  return 0;
}

void test_StringRef() {
  errs() << "\n\n===" << __PRETTY_FUNCTION__ << "\n";
  StringRef ref("hello");
  errs() << ref << "\n";
  hash_code code = hash_value(ref);
  errs() << "hash: " << code << '\n';
  StringRef lh, rh;
  std::tie(lh, rh) = ref.split('l');
  errs() << lh << '\t' << rh << '\n';
  std::tie(lh, rh) = ref.split(StringRef("ll"));
  errs() << lh << '\t' << rh << '\n';
}

void test_Twine(bool verbose) {
  errs() << "\n\n===" << __PRETTY_FUNCTION__ << "(" << verbose << ")\n";
  StringRef sr = "hello world";
  std::string ss = "s";
  Twine t_e;
  Twine t_sr(sr);
  Twine t_s(ss);
  char c = 'c';
  unsigned ii = 12345;
  Twine t_c(c);
  Twine t_ii(ii);
  Twine t_src = t_sr + t_c;
  Twine t_srcs = t_src + t_s;
  Twine t__csr("HW", StringRef("!"));
  Twine t__src(StringRef("HI"), "?");
  auto st = llvm::SmallVector<char, 3>();
  st.push_back('h');
  st.push_back('i');
  st.push_back('!');
  Twine t_st = Twine(st);
#define X printRepr
#define dumpWithNewLine(twine) \
  errs() << #twine << '\t';    \
  twine.X(errs());             \
  if (twine.isSingleStringRef()) { \
    errs()<<"\t>>>" << twine.getSingleStringRef()<<"<<<\n"; \
  } else {                     \
    errs() << "\tMULTIPLE\n";  \
  }
  dumpWithNewLine(t_e);
  dumpWithNewLine(t_sr);
  dumpWithNewLine(t_s);
  dumpWithNewLine(t_c);
  dumpWithNewLine(t_ii);
  dumpWithNewLine(t_src);
  dumpWithNewLine(t_srcs);
  dumpWithNewLine(t__csr);
  dumpWithNewLine(t__src);
  dumpWithNewLine(t_st);
#undef dumpWithNewLine
}

void test_ArrayRef() {
  errs() << "\n\n===" << __PRETTY_FUNCTION__ << "\n";
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
  errs() << "\n\n===" << __PRETTY_FUNCTION__ << "\n";
  std::vector<StringRef> strs{"hello", "World", "nihao", "!",
                              "jiejie", "how", "none"};
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
  test_md5();
  test_format();
  test_StringRef();
  takeCallBack(test_Twine, true);
  test_ArrayRef();
  test_StringSwitch();
  test_PackedVector();
  test_rtti();
  test_yaml();
  return 0;
}
