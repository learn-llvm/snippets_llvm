#define DEBUG_TYPE "TestGlobalValue"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"

#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include "Logging.hh"
#include "Utilities.hh"

using namespace llvm;

namespace {

struct TestGlobalValue final : public ModulePass {
  static char ID;

  TestGlobalValue() : ModulePass(ID) {}

  bool runOnModule(Module &M) override {
    errs() << getPassName() << " ID:" << getPassID() << "\n";
    dumpPassKind(getPassKind());
    dumpPassStructure();
    for (auto &gv : M.getGlobalList()) {
      dumpGVInfo(gv);
    }
    for (auto &fn : M) {
      dumpGVInfo(fn);
    }
    for (auto &ga : M.getAliasList()) {
      dumpGVInfo(ga);
    }
    return false;
  }

    void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }
};

char TestGlobalValue::ID = 0;
static RegisterPass<TestGlobalValue> X("TestGlobalValue", "TestGlobalValue",
                                       true, true);
}
