#include "Common.hh"
#include <cstdlib>

std::shared_ptr<char> cppDemangle(const char *abiName) {
  int status;
  char *ret = abi::__cxa_demangle(abiName, 0, 0, &status);

  /* NOTE: must free() the returned char when done with it! */
  std::shared_ptr<char> retval;
  retval.reset(ret, [](char *mem) {
    if (mem) std::free((void *)mem);
  });
  return retval;
}
