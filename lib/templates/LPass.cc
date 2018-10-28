#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"

using namespace llvm;

namespace llvm {
class LPass : public LoopPass {
 private:
 public:
  static char ID;
  LPass() : LoopPass(ID) {}

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
    AU.addRequired<LoopInfo>();
    AU.addPreserved<LoopInfo>();
  }

  bool runOnLoop(Loop *L, LPPassManager &LP) {
    for (BasicBlock const *B : L->getBlocks()) {
      errs() << " " << B->getName();
    }
    return false;
  }
};
}  // namespace llvm

char LPass::ID = 0;
static RegisterPass<LPass> X("LPass", "LPass pass", true, true);
