#ifndef FNUTILS_H
#define FNUTILS_H

#include "llvm/ADT/Twine.h"
#include "llvm/IR/IRBuilder.h"

namespace llvm {
class CallInst;
class Constant;
class Function;
class GlobalVariable;
class Instruction;
class Module;
class Type;

namespace utils {
CallInst* getAssertCallSite(Function* func);
Constant* geti8StrVal(Module& M, char const* str, Twine const& name = ".str");
Function* getFn_exit(Module& M);
Function* getFn_kleeMakeSymbolic(Module& M);
Function* getFn_assert(Module& mod);
void sym_Vars(Value* sym_addr, BasicBlock* parent, IRBuilder<>& builder);
void sym_Vars(Value* sym_addr, BasicBlock* parent,
              BasicBlock::iterator insertPtr, IRBuilder<>& builder);
}  // namespace llvm::utils
}  // namespace llvm

#endif
