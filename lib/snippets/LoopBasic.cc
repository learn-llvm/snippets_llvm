#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

typedef SmallVector<BasicBlock *, 4> BlockVectTy;

namespace llvm {

struct LoopBasic : public FunctionPass {
  static char ID;
  LoopBasic() : FunctionPass(ID) {}
  virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    AU.addRequired<LoopInfo>();
    AU.addPreserved<LoopInfo>();
  }

  bool runOnLoop(Loop &L) {
    {
      auto blocks = L.getBlocks();
      errs() << "[blocks]:\n";
      unsigned sz = blocks.size();
      for (unsigned i = 0; i < sz; ++i) {
        auto *B = blocks[i];
        errs() << "i=" << i + 1 << ", " << B->getName() << "\n";
      }
      errs() << '\n';
    }
    {
      errs() << "ExitBlocks:\n";
      BlockVectTy exitBlocks;
      L.getExitBlocks(exitBlocks);
      for (auto &block : exitBlocks) errs() << block->getName() << "\t";
      errs() << "\n\n";
    }
    {
      errs() << "getLoopPreheader=" << L.getLoopPredecessor()->getName()
             << '\n';
      errs() << "getLoopPredecessor=" << L.getLoopPreheader()->getName()
             << '\n';
      errs() << "getLoopLatch=" << L.getLoopPreheader()->getName() << '\n';
      errs() << '\n';
    }
    {
      auto subLoops = L.getSubLoopsVector();
      errs() << "[subLoops] size=" << subLoops.size() << '\n';
      for (unsigned i = 0; i < subLoops.size(); ++i) {
        errs() << "[i=" << i + 1 << "]: ";
        for (auto *B : subLoops[i]->getBlocks()) {
          errs() << B->getName() << " ";
        }
        errs() << '\n';
      }
    }
    return false;
  }

  bool runOnFunction(Function &F) override {
    errs() << "===> Function=" << F.getName() << " <===\n";
    LoopInfo &info = getAnalysis<LoopInfo>();
    for (auto &loop : info) {
      runOnLoop(*loop);
    }
    return false;
  }
};

char LoopBasic::ID = 0;
static RegisterPass<LoopBasic> X("LoopBasic", "LoopBasic", true, true);
};
