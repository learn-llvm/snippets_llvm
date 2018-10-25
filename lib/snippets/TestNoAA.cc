#define DEBUG_TYPE "TestNoAA"
#include "Version.hh"

#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/TypeBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IRReader/IRReader.h"

#if LLVM_VERSION_CODE >= LLVM_VERSION(3, 5)
#include "llvm/IR/Verifier.h"
#else
#include "llvm/Analysis/Verifier.h"
#endif

#include "llvm/Pass.h"
#include "llvm/PassManager.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/TargetSelect.h"

#include "Logging.hh"
#include "Utilities.hh"

using namespace llvm;

namespace {

struct TestNoAA final : public FunctionPass {
  static char ID;

  TestNoAA() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override {
    AliasAnalysis &AA = getAnalysis<AliasAnalysis>();
    for (auto &A : F.getArgumentList()) {
      logging::prettyPrint(&A);
      errs() << "\tisNoAliasArgument: " << isNoAliasArgument(&A) << "\n";
    }
    for (auto &B : F) {
      for (auto &I : B) {
        if (isa<InvokeInst>(&I) || isa<CallInst>(&I)) {
          logging::prettyPrint(&I);
          errs() << "\tisNoAliasCall:" << isNoAliasCall(&I) << "\n";
        }
      }
    }
    return false;
  }

  virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    AU.addRequired<AliasAnalysis>();
  }
};

char TestNoAA::ID = 0;
static RegisterPass<TestNoAA> X("TestNoAA", "TestNoAA", true, true);
}
