#define DEBUG_TYPE "IR"

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"

#include "llvm/ADT/SetVector.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_os_ostream.h"

#include "llvm/IR/DataLayout.h"

#include "LLDump.hh"
#include "LLUtils.hh"

using namespace llvm;

namespace llvm {

namespace detail {
uint64_t getTypeSize(DataLayout const &targetData, Type *type) {
  if (type->isFunctionTy())  // not sized
    return targetData.getPointerSize();
  if (!type->isSized())
    return 100;  // FIXME hard code
  if (auto *structType = dyn_cast<StructType>(type))
    return targetData.getStructLayout(structType)->getSizeInBytes();
  return targetData.getTypeAllocSize(type);
}
}  // namespace detail

namespace utils {

bool isTrivialPointer(Value const *V) {
  if (isa<AllocaInst>(V) || isa<GlobalVariable>(V) || isa<Function>(V)) {
    assert(V->getType()->isPointerTy());
    return true;
  }
  return false;
}

bool isConstantValue(Value const *V) {
  return isa<Constant>(V) && !isa<GlobalValue>(V) && !isa<UndefValue>(V);
}

bool isPointerValue(Value const *V) {
  if (isTrivialPointer(V))
    return cast<PointerType const>(V->getType())
        ->getNonOpaquePointerElementType()
        ->isPointerTy();
  return V->getType()->isPointerTy();
}

bool isPointerToPointerValue(Value const *V) {
  return isPointerValue(V) && getPointedType(V)->isPointerTy();
}

/// REVIEW: what is called pointer manipulation?
bool isPointerManipulation(Instruction const *I) {
  if (isa<AllocaInst>(I)) {
    return false;
  } else if (isa<LoadInst>(I)) {  /// PointerOperand's element type
    if (dyn_cast<PointerType const>(I->getOperand(0)->getType())
        ->getNonOpaquePointerElementType()
        ->isPointerTy())
      return true;
  } else if (isa<StoreInst>(I)) {  /// ValueOperand type
    if (I->getOperand(0)->getType()->isPointerTy())
      return true;
  } else if (isa<BitCastInst>(I)) {  /// ty may vary, bits preserve
    if (I->getType()->isPointerTy() &&
        I->getOperand(0)->getType()->isPointerTy())
      return true;                         /// REVIEW why &&?
  } else if (isa<GetElementPtrInst>(I)) {  /// REVIEW always true?
    return true;
  } else if (auto const *C = dyn_cast<CallInst>(I)) {
    if (C->isInlineAsm())
      return false;
    return isFn_mem_ops(C->getCalledFunction());
  } else if (auto const *PHI = dyn_cast<PHINode>(I)) {
    return isPointerValue(PHI);
  } else if (auto const *EV = dyn_cast<const ExtractValueInst>(I)) {
    return isPointerValue(EV);
  } else if (auto const *IV = dyn_cast<const InsertValueInst>(I)) {
    return isPointerValue(IV->getInsertedValueOperand());
  } else if (isa<IntToPtrInst>(I)) {  /// inttoptr
    return true;
  } else if (auto const *SEL = dyn_cast<SelectInst>(I)) {
    if (isPointerValue(SEL))
      return true;
  }

  if (isPointerValue(I)) {
    WITH_COLOR(raw_ostream::RED, errs() << *I << "\n");
    report_fatal_error("Instruction can't be a Pointer Value");
  }
  return false;
}

Type const *getPointedType(Value const *V) {
  const Type *ty = getPointedType(V->getType());
  if (isTrivialPointer(V))
    ty = getPointedType(ty);
  return ty;
}

Type const *getPointedType(Type const *T) {
  return cast<PointerType>(T)->getNonOpaquePointerElementType();
}

/// REVIEW possibably wrong
bool isGlobalPointerInit(GlobalVariable const *G) {
  if (G->isDeclaration())
    return false;
  Value const *op = G->getOperand(0);
  /// TODO this assert is only for debug use
  if (isa<Function>(G)) {
    assert(cast<PointerType const>(op->getType())
               ->getNonOpaquePointerElementType()
               ->isFunctionTy());
  }
  return isTrivialPointer(op);
}

/// REVIEW callback is treated as usual
FunctionType const *getCalleeFnType(CallInst const *C) {
  assert(!C->isInlineAsm() && "Inline assembly is not supported!");
  if (auto *fn = dyn_cast<Function>(C->getCalledFunction()))
    return fn->getFunctionType();
  else {  /// function pointer
    Type const *fnT =
        dyn_cast<PointerType const>(C->getCalledFunction()->getType())
            ->getNonOpaquePointerElementType();
    return dyn_cast<FunctionType const>(fnT);
  }
}

/// REVIEW whether isDeclaration check is suitable for different runtime?
bool isFn_malloc(Function const *F) {
  return F->isDeclaration() && F->hasName() && F->getName().equals("malloc");
}

bool isFn_free(Function const *F) {
  return F->isDeclaration() && F->hasName() && F->getName().equals("free");
}

/// REVIEW: intrinsic functions always has external linkage
bool isFn_memcpy(Function const *F) {
  return F->getIntrinsicID() == Intrinsic::memcpy;
}

bool isFn_memmove(Function const *F) {
  return F->getIntrinsicID() == Intrinsic::memmove;
}

bool isFn_memset(Function const *F) {
  return F->getIntrinsicID() == Intrinsic::memset;
}

bool isFn_mem_ops(Function const *F) {
  if (!F)
    return false;
  return isFn_malloc(F) || isFn_free(F) || isFn_memcpy(F) || isFn_memmove(F) ||
      isFn_memset(F);
}

bool is_inlineAsm_withSideEffect(CallInst const *C) {
  if (auto *a = dyn_cast<InlineAsm>(C->getCalledFunction()))
    return a->hasSideEffects();
  return false;
}

bool isInst_mem_ops(CallInst const *C) {
  return isFn_mem_ops(C->getCalledFunction());
}

Instruction const *getFunctionEntry(Function const *F) {
  return &F->getEntryBlock().front();
}

/// TODO Argument, MDNode?
bool isLocalToFunction(Value const *V, Function const *F) {
  if (auto *I = dyn_cast<Instruction>(V))
    return I->getParent()->getParent() == F;
  return false;
}

bool callToVoidFunction(CallInst const *C) {
  if (C->isInlineAsm())
    return false;  /// exclude inlineAsm
  return C->getType()->getTypeID() == Type::VoidTyID;
}

Instruction const *getSuccInBlock(Instruction const *I) {
  BasicBlock::const_iterator it(I);
  /// if succ doesn't exist in bb, return NULL
  ++it;
  return it == I->getParent()->end() ? nullptr : &*it;
}

Instruction const *getPredInBlock(Instruction const *I) {
  BasicBlock::const_iterator it(I);
  Instruction const *retPtr =  /// TODO check whether it's a UD
      (it == I->getParent()->begin()) ? nullptr : &*(--it);
  return retPtr;
}

/// GEP,CAST ConstantExpr needs to use its first operand
Value *elimConstExpr(Value *V) {
  if (auto *CE = dyn_cast<ConstantExpr>(V)) {
    if (Instruction::isBinaryOp(CE->getOpcode()))
      return V;  /// itself
    assert((CE->getOpcode() == Instruction::GetElementPtr || CE->isCast()) &&
        "Only GEP or CAST supported");
    return elimConstExpr(CE->getOperand(0));
  }
  return V;  /// don't change if not ConstantExpr
}

Value const *elimConstExpr(Value const *V) {
  return elimConstExpr(const_cast<Value *>(V));
}

/// int and int*, or their combination(array/struct), klee can deal with
bool isIntegerRelatedType(Type const *ty) {
  if (ty->isPointerTy()) {  // int*
    return isa<IntegerType>((cast<PointerType>(ty))->getNonOpaquePointerElementType());
  } else if (ty->isStructTy()) {  // struct with int/int* or recursively
    auto *structTy = cast<StructType>(ty);
    for (unsigned i = 0; i < structTy->getNumElements(); ++i) {
      Type const *eleTy = structTy->getElementType(i);
      if (!isIntegerRelatedType(eleTy))
        return false;  // one ele isn't, it isn't
    }
    return true;
  } else if (ty->isArrayTy()) {  // array with int/int* or recursively
    return isIntegerRelatedType((cast<ArrayType>(ty))->getElementType());
  }
  return isa<IntegerType>(ty);  // int
}

/// -----------------------------------------------------

SmallVector<CallInst *, 4> getAssertCallSite(Function *func) {
  SmallVector<CallInst *, 4> calls;
  for (BasicBlock &B : *func) {
    for (Instruction &I : B) {
      if (auto *callInst = dyn_cast<CallInst>(&I)) {
        Function const *callee = callInst->getCalledFunction();
        if (callee && callee->getName() == "__assert_fail") {
          calls.push_back(callInst);
        }
      }
    }
  }
  return calls;
}

Constant *geti8StrVal(Module &M, char const *str, Twine const &name) {
  Constant *strConstant = ConstantDataArray::getString(M.getContext(), str);
  auto *GVStr =
      new GlobalVariable(M, strConstant->getType(), true,
                         GlobalValue::InternalLinkage, strConstant, name);
  Constant *zero =
      Constant::getNullValue(IntegerType::getInt32Ty(M.getContext()));
  Constant *indices[] = {zero, zero};
  // not sure
  Constant *strVal = ConstantExpr::getGetElementPtr(strConstant->getType(),
                                                    GVStr, indices, true);
  return strVal;
}

Function *getFn_exit(Module &M) {
  return nullptr;
}

}  // namespace utils

}  // namespace llvm
