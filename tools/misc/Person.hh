//
// Created by hongxu on 10/28/18.
//

#ifndef PROJECT_PERSON_HH
#define PROJECT_PERSON_HH

#include <string>
#include "llvm/Support/YAMLParser.h"
#include "llvm/Support/YAMLTraits.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormatProviders.h"

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
    if (index >= persons.size()) {
      report_fatal_error("out of bound", true);
    }
    return persons[index];
  }
};

template<>
struct format_provider<Person> {
  static void format(const Person &p, raw_ostream &os, StringRef style) {
    os << "Person(name=" << p.name_ << ", age=" << p.age_ << ")";
  }
};

#endif //PROJECT_PERSON_HH
