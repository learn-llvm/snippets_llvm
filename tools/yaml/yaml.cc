#define DEBUG_TYPE "MyYaml"
#include "llvm/Support/YAMLParser.h"
#include "llvm/Support/YAMLTraits.h"

#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

struct Person {
  std::string name_;
  unsigned age_;
  Person(std::string const &name, unsigned age = 0) : name_(name), age_(age) {}
};

template <>
struct yaml::MappingTraits<Person> {
  static void mapping(yaml::IO &io, Person &p) {
    io.mapRequired("name", p.name_);
    io.mapOptional("age", p.age_);
  }
};

template <>
struct yaml::SequenceTraits<std::vector<Person>> {
  static size_t size(IO &io, std::vector<Person> &persons) {
    return persons.size();
  }
  static Person &element(yaml::IO &, std::vector<Person> &persons,
                         size_t index) {
    if (index >= persons.size()) assert(0 && "out of bound");
    return persons[index];
  }
};

int main(void) {
  Person p1("hello", 1);
  Person p2("world");
  std::vector<Person> persons;
  persons.push_back(p1);
  persons.push_back(p2);
  yaml::Output yout(llvm::errs());
  yout << persons;
  return 0;
}
