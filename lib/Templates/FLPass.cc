#define DEBUG_TYPE "FLPass"
#include "llvm/Support/Debug.h"

#include "Version.hh"
#if LLVM_VERSION_CODE >= LLVM_VERSION(3, 3)
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#else
#include "llvm/Function.h"
#include "llvm/BasicBlock.h"
#include "llvm/Instructions.h"
#endif

#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include "Logging.hh"

using namespace llvm;

struct FLPass final : public FunctionPass {
  static char ID;
  FLPass() : FunctionPass(ID) {}
  virtual void print(llvm::raw_ostream &O, const Module *M) const override {
    errs() << __PRETTY_FUNCTION__ << "\n";
  }

  bool runOnFunction(Function &F) override { return false; }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }
};

char FLPass::ID = 0;
static RegisterPass<FLPass> X("FLPass", "FLPass pass", true, true);
