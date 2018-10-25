#ifndef DBG_LINE_INFO_H
#define DBG_LINE_INFO_H

#include "llvm/ADT/DenseMap.h"

using namespace llvm;

namespace llvm {

class DbgDeclareInst;
class User;
class Function;
class BasicBlock;
class Instruction;
class GlobalVariable;
class Module;
class Value;

namespace dbg {

struct VarInfo {
  VarInfo(std::string displayName_, unsigned lineNo_)
      : displayName(displayName_), lineNo(lineNo_) {};
  std::string displayName;
  unsigned lineNo;
};

typedef DenseMap<User const *, VarInfo> VarDeclMap;
Value *findDbgGlobalDeclare(GlobalVariable *V);
DbgDeclareInst const *findDbgDeclare(Value const *V);
bool getDbgDeclareInfo(const Value *V, std::string &DisplayName,
                       unsigned &LineNo);
std::string getLocalVarName(Instruction const *ins);
unsigned getSrcLineFromDeclare(DbgDeclareInst const *DDI);
bool getVarDeclMap(Function &F, VarDeclMap &varDeclMap);
bool getVarDeclMap(BasicBlock &B, VarDeclMap &varDeclMap);
bool getGlobalDeclMap(Module const &M, VarDeclMap &gvLocMap);
unsigned getLineNum(Instruction const *inst);

}  /// namespace llvm::dbg
}  /// namespace llvm

#endif
