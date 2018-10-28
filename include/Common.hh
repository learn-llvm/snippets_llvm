#ifndef COMMON_HPP
#define COMMON_HPP

#include <memory>

#define COUNT_OF(x) \
  ((sizeof(x) / sizeof(0 [x])) / ((size_t)(!(sizeof(x) % sizeof(0 [x])))))

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(TypeName const &) = delete;     \
  TypeName &operator=(TypeName const &) = delete;

#define DISALLOW_ASSIGN(TypeName) \
  TypeName &operator=(TypeName const &) = delete;

#include <cxxabi.h>  // needed for abi::__cxa_demangle

std::shared_ptr<char> cppDemangle(const char *abiName);
#define CLASS_NAME(somePointer) ((const char *) cppDemangle(typeid(*somePointer).name()).get() )

#endif
