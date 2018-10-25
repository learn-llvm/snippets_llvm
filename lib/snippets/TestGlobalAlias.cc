#define DEBUG_TYPE "TestGlobalAlias"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"

#include "llvm/Pass.h"
#include "Utilities.hh"

using namespace llvm;

namespace {

struct TestGlobalAlias final : public ModulePass {
  static char ID;

  TestGlobalAlias() : ModulePass(ID) {}

  void dumpGAInfo(GlobalAlias &ga) {
    WITH_COLOR(raw_ostream::YELLOW, errs() << "globalalias: ");
    logging::prettyPrint(&ga);
    dumpLinkageType(ga);
#if LLVM_VERSION_CODE >= LLVM_VERSION(3, 5)
    GlobalObject *gv = ga.getBaseObject();
#else
    GlobalValue *gv = ga.getAliasedGlobal();
#endif
    if (gv) {
      WITH_COLOR(raw_ostream::YELLOW, errs() << "aliasee: ");
      logging::prettyPrint(gv);
      dumpLinkageType(*gv);
    }
  }

  bool runOnModule(Module &M) {
    for (auto &ga : M.getAliasList()) {
      dumpGAInfo(ga);
    }
    return false;
  }

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
  }
};

char TestGlobalAlias::ID = 0;
static RegisterPass<TestGlobalAlias> X("TestGlobalAlias", "TestGlobalAlias",
                                       true, true);
}
