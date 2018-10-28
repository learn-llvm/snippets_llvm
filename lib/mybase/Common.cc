#include "Common.hh"

std::shared_ptr<char> cppDemangle(const char *abiName) {
  int status;
  char *ret = abi::__cxa_demangle(abiName, 0, 0, &status);

  /* NOTE: must free() the returned char when done with it! */
  std::shared_ptr<char> retval;
  retval.reset((char *) ret, [](char *mem) {
    if (mem)
      free((void *) mem);
  });
  return retval;
}