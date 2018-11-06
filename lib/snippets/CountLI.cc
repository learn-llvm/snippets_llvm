//
// Created by hongxu on 11/6/18.
//

#include "llvm/IR/Function.h"

#include "llvm/Pass.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/raw_os_ostream.h"

#include "llvm/IR/CFG.h"

using namespace llvm;

class CountLD : public FunctionPass {
public:
  static char ID;

  CountLD() : FunctionPass(ID) {}

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    AU.addRequired<LoopInfoWrapperPass>();
  }

  virtual bool runOnFunction(Function &F) override {
    errs() << "\nFunc: " << F.getName() << "\n";
    LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    if (LI.empty()) {
      errs() << "no loop inside\n";
    } else {
      for (BasicBlock &B: F) {
        errs() << "BB: " << B.getName() << "\n";
        Loop *li = LI.getLoopFor(&B);
        if (li != nullptr) {
          auto predBB = li->getLoopPreheader();
          if (predBB != nullptr) {
            errs() << "\tpred:" << predBB->getName() << "\n";
          }
          auto preHeader = li->getLoopPreheader();
          if (preHeader != nullptr) {
            errs() << "\tpreheader:" << preHeader->getName() << "\n";
          }
        }
      }
    }
    return false;
  }

};

char CountLD::ID = 0;

static RegisterPass<CountLD> X("count-li", "count loop info", true, true);