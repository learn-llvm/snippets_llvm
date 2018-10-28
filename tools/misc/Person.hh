//
// Created by hongxu on 10/28/18.
//

#ifndef PROJECT_PERSON_HH
#define PROJECT_PERSON_HH

#include <string>
#include "llvm/Support/YAMLParser.h"
#include "llvm/Support/YAMLTraits.h"

using namespace llvm;

struct Person {
  std::string name_;
  unsigned age_;
  Person(std::string const &name, unsigned age = 0) : name_(name), age_(age) {}
};

template<>
struct yaml::MappingTraits<Person> {
  static void mapping(yaml::IO &io, Person &p) {
    io.mapRequired("name", p.name_);
    io.mapOptional("age", p.age_);
  }
};

template<>
struct yaml::SequenceTraits<std::vector<Person>> {
  static size_t size(IO &io, std::vector<Person> &persons) {
    return persons.size();
  }
  static Person &element(yaml::IO &, std::vector<Person> &persons,
                         size_t index) {
    if (index >= persons.size())
      assert(0 && "out of bound");
    return persons[index];
  }
};

#endif //PROJECT_PERSON_HH
