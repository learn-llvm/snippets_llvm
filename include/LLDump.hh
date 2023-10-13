#ifndef HELPERS_H
#define HELPERS_H

#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/PassManager.h"
#include <string>

namespace llvm {
class Instruction;
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

std::string getTypeStr(Type const *type);
std::string getValueStr(Value const *);

template<typename T>
static std::string ToString(const T *obj) {
  std::string TypeName;
  raw_string_ostream N(TypeName);
  obj->print(N);
  return N.str();
}

std::string InstTypeStr(char const *instTypeChars);

void dumpLinkageType(llvm::GlobalValue &GV);
void dumpGVInfo(llvm::GlobalValue &GV);
void dumpPassKind(PassKind kind);

static inline StringRef ppName(StringRef name) {
  if (name.empty()) {
    return "<NONE>";
  } else {
    return name;
  }
}

void printInstKind(Instruction const &);
void prettyPrint(Value const *V, unsigned endLine = 0, unsigned startLine = 0);

enum RB_Diagnostics {
  LOG,
  WARN,
  ERROR,
  FATAL
};

inline void rbscope_diagnostics(RB_Diagnostics level, char const *msg) {
  switch (level) {
    case RB_Diagnostics::LOG: WITH_COLOR(raw_ostream::CYAN, errs() << "LOG: " << msg << "\n");
      break;
    case RB_Diagnostics::WARN: WITH_COLOR(raw_ostream::YELLOW, errs() << "WARN: " << msg << "\n");
      break;
    case RB_Diagnostics::ERROR: WITH_COLOR(raw_ostream::MAGENTA, errs() << "ERROR: " << msg << "\n");
      abort();
      break;
    case RB_Diagnostics::FATAL: WITH_COLOR(raw_ostream::RED, errs() << "FATAL: " << msg << "\n");
      abort();
      break;
  }
}

}  // namespace llvm
#endif
