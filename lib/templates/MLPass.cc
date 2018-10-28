#define DEBUG_TYPE "MLPass"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"

#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include "LLDump.hh"

using namespace llvm;

namespace {

struct MLPass final : public ModulePass {
  static char ID;

  MLPass() : ModulePass(ID) {}

  bool runOnModule(Module &M) override { return false; }

  virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }
};

char MLPass::ID = 0;
static RegisterPass<MLPass> X("MLPass", "MLPass pass", true, true);
}  // namespace
