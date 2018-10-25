#ifndef HELPERS_H
#define HELPERS_H

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Constants.h"

namespace llvm {
class Instruction;
class MemDepResult;
class Type;
class Value;
class Function;
class Module;
}

#define WITH_COLOR(color, x)         \
  {                                  \
    llvm::errs().changeColor(color); \
    x;                               \
    llvm::errs().resetColor();       \
  }

#define BEG_FUN_LOG()                                                      \
  DEBUG(llvm::errs() << "--- beg [" << __FUNCTION__ << "] in " << __FILE__ \
                     << " " << DEBUG_TYPE << "--- \n")
#define END_FUN_LOG()                                                      \
  DEBUG(llvm::errs() << "--- end [" << __FUNCTION__ << "] in " << __FILE__ \
                     << " " << DEBUG_TYPE << "--- \n")

namespace llvm {
namespace logging {

void printTypeInfo(Type const* type);
void printValueInfo(Value const*);

std::string InstTypeStr(char const* instTypeChars);

void printInstKind(Instruction const&);
void prettyPrint(Value const* V, unsigned endLine = 0, unsigned startLine = 0);

enum RB_Diagnostics {
  LOG,
  WARN,
  ERROR,
  FATAL
};

extern "C" void abort(void);
inline void rbscope_diagnostics(RB_Diagnostics level, char const* msg) {
  switch (level) {
    case RB_Diagnostics::LOG:
      WITH_COLOR(raw_ostream::CYAN, errs() << "LOG: " << msg << "\n");
      break;
    case RB_Diagnostics::WARN:
      WITH_COLOR(raw_ostream::YELLOW, errs() << "WARN: " << msg << "\n");
      break;
    case RB_Diagnostics::ERROR:
      WITH_COLOR(raw_ostream::MAGENTA, errs() << "ERROR: " << msg << "\n");
      abort();
      break;
    case RB_Diagnostics::FATAL:
      WITH_COLOR(raw_ostream::RED, errs() << "FATAL: " << msg << "\n");
      abort();
      break;
  }
}

}  // namespace llvm::logging
}  // namespace llvm
#endif
