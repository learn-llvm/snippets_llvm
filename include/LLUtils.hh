#ifndef IR_UTILS_H
#define IR_UTILS_H

#include "llvm/ADT/Twine.h"
#include "llvm/IR/IRBuilder.h"

namespace llvm {
class CallInst;
class Constant;
class Function;
class FunctionType;
class GlobalVariable;
class Instruction;
class Module;
class DataLayout;
class Type;
class Value;

namespace utils {

static inline void visitBB(BasicBlock &B, function_ref<bool(Instruction &)> cb) {
  for (auto &I : B) {
    if (cb(I)) {
      errs() << I << " meets cb requirement\n";
    }
  }
}

CallInst *getAssertCallSite(Function *func);
Constant *geti8StrVal(Module &M, char const *str, Twine const &name = ".str");
Function *getFn_exit(Module &M);
Function *getFn_kleeMakeSymbolic(Module &M);
Function *getFn_assert(Module &mod);
void sym_Vars(Value *sym_addr, BasicBlock *parent, IRBuilder<> &builder);
void sym_Vars(Value *sym_addr, BasicBlock *parent,
              BasicBlock::iterator insertPtr, IRBuilder<> &builder);

bool isTrivialPointer(Value const * V);
bool isConstantValue(Value const * V);
bool isPointerValue(Value const * V);
bool isPointerToPointerValue(Value const * V);
bool isPointerManipulation(Instruction const * I);
Type const *getPointedType(Value const * V);
Type const *getPointedType(Type const *T);
bool isGlobalPointerInit(GlobalVariable const * G);
FunctionType const *getCalleeFnType(CallInst const * C);
bool isFn_malloc(Function const *F);
bool isFn_free(Function const * F);
bool isFn_memcpy(Function const * F);
bool isFn_memmovej(Function const * F);
bool isFn_memset(Function const * F);
bool isFn_mem_ops(Function const * F);
bool is_inlineAsm_withSideEffect(CallInst const *CI);
bool isInst_mem_ops(CallInst const * C);
Instruction const *getFunctionEntry(Function const *F);
bool isLocalToFunction(Value const * V, Function const * F);
bool callToVoidFunction(CallInst const * C);
Instruction const *getSuccInBlock(Instruction const *);
Instruction const *getPredInBlock(Instruction const *);
Value *elimConstExpr(Value *V);
Value const *elimConstExpr(const Value *V);
unsigned getTypeSize(DataLayout const &targetData, Type const *type);
bool isIntegerRelatedType(Type const *ty);
}  //  namespace llvm::utils
}  // namespace llvm

#endif
