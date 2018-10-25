#define DEBUG_TYPE "TestOF"
#include "llvm/Support/Debug.h"

#include "llvm/IR/Function.h"

#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/DominanceFrontier.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicInst.h"

#include "Logging.hh"

using namespace llvm;

struct TestOF final : public FunctionPass {
  static char ID;
  TestOF() : FunctionPass(ID) {}

  void printOF(Function &F) {
    for (auto &B : F) {
      for (auto &I : B) {
        printInstArgType(I);
        if (IntrinsicInst *II = dyn_cast<IntrinsicInst>(&I)) {
          errs() << "inst: " << *II << "\n";
          logging::printTypeInfo(II->getType());
        }
      }
    }
  }

  void printInstArgType(Instruction &I) {
    errs() << "\nInstruction: " << I << "\n";
    Type *instTy = I.getType();
    if (instTy->isPointerTy()) {
      instTy = dyn_cast<PointerType>(instTy)->getElementType();
    }
    logging::printTypeInfo(instTy);
    for (auto &arg : I.operands()) {
      Type *ty = arg->getType();
      if (ty->isPointerTy()) {
        ty = dyn_cast<PointerType>(ty)->getElementType();
      }
      logging::printTypeInfo(ty);
    }
  }

  bool runOnFunction(Function &F) override {
    printOF(F);
    return false;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    AU.addRequired<PostDominatorTree>();
  }
};

char TestOF::ID = 0;
static RegisterPass<TestOF> X("TestOF", "TestOF pass", true, true);
