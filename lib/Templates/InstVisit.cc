#define DEBUG_TYPE "InstVisitTest"
#include "llvm/Support/Debug.h"

#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/InstVisitor.h"

#include "Logging.hh"

using namespace llvm;

namespace {
struct CountAllocaVisitor : public InstVisitor<CountAllocaVisitor> {
  unsigned Count;
  CountAllocaVisitor() : Count(0) {}
};

void CountAllocaVisitor::visitAllocaInst(AllocaInst &AI) { ++Count; }

struct InstVisitTest final : public FunctionPass {
  static char ID;
  InstVisitTest() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) {
    errs() << F.getName() << "\n";
    CountAllocaVisitor CAV;
    CAV.visit(F);
    errs() << "allocaInst num:" << CAV.Count << "\n";
    return false;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const { AU.setPreservesAll(); }
};
}

char InstVisitTest::ID = 0;
static RegisterPass<InstVisitTest> X("InstVisitTest", "InstVisitTest pass",
                                     true, true);
