#define DEBUG_TYPE "PrintDT"
#include "llvm/Support/Debug.h"

#include "llvm/IR/Function.h"

#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/DominanceFrontier.h"

#include "Logging.hh"

using namespace llvm;

struct PrintDT final : public FunctionPass {
  static char ID;
  PrintDT() : FunctionPass(ID) {}

  void printDT(Function &F) {
    errs() << "\n";
    logging::prettyPrint(&F);
    PostDominatorTree &PDT = getAnalysis<PostDominatorTree>();
    for (BasicBlock &B : F) {
      errs() << "\n";
      logging::prettyPrint(&B);
      errs() << "\t";
      DomTreeNode *node = PDT.getNode(&B);
      if (!node) {
        errs() << " no node\n";
      } else {
        DomTreeNode *idomNode = node->getIDom();
        if (idomNode) {
          BasicBlock *idomBB = idomNode->getBlock();
          if (idomBB) {
            errs() << idomBB->getName() << "\t";
          } else {
            errs() << "no idomBB\n";
          }
        } else {
          errs() << "no idom\n";
        }
      }  /// end of node check
    }    /// end of B loop
  }

  bool runOnFunction(Function &F) override {
    printDT(F);
    return false;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    AU.addRequired<PostDominatorTree>();
  }
};

char PrintDT::ID = 0;
static RegisterPass<PrintDT> X("PrintDT", "PrintDT pass", true, true);
