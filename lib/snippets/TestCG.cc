#define DEBUG_TYPE "TestCG"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Support/YAMLParser.h"
#include "llvm/Support/YAMLTraits.h"

#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include "Logging.hh"

#define AGGRESSIVE_CALLEE 1

using namespace llvm;

namespace {

using FnVect = std::vector<Function *>;

struct TestCG final : public ModulePass {
  static char ID;

  TestCG() : ModulePass(ID) {}

  bool isCallSite(Instruction *I) {
    return isa<CallInst>(I) || isa<InvokeInst>(I);
  }

  FnVect findPossibleCallee(Value *callee) {
    FnVect fnVect;
    if (Instruction *inst = dyn_cast<Instruction>(callee)) {
      Type *ty = callee->getType();
      assert(ty->isPointerTy());
      Type *calleeTy = cast<PointerType>(ty)->getElementType();
      Function *currentFn = inst->getParent()->getParent();
      Module *module = currentFn->getParent();
      for (auto &fn : *module) {
        if (fn.getFunctionType() == calleeTy) {
#if AGGRESSIVE_CALLEE
          fnVect.push_back(&fn);
// #else
//           for (auto *U : fn.users()) {
//             if (Instruction *fnUser = dyn_cast<Instruction>(U)) {
//               if (fnUser->getParent()->getParent() == currentFn) {
//                 fnVect.push_back(&fn);
//               }
//             }
//           }
#endif
        }
      }
    } else if (Argument *arg = dyn_cast<Argument>(callee)) {
    } else {
      assert(0 && "not yet");
    }
    return fnVect;
  }

  bool runOnModule(Module &M) override {
    CallGraph cg(M);
    /// refineCG(M, cg);
    CallGraphNode *externCaller = cg.getExternalCallingNode();
    /// externCaller->removeAllCalledFunctions();
    CallGraphNode *externCallee = cg.getCallsExternalNode();
    errs() << externCaller << " " << externCallee
           << (externCaller == externCallee) << "\n";
    if (externCallee) {
      errs() << "cs number: " << externCallee->getNumReferences() << "\n";
    }
    for (auto &fnMap : cg) {
      CallGraphNode *cgn = fnMap.second;
      Function *fn = cgn->getFunction();
      if (fn) {
        errs() << "\nfunction: " << fn->getName() << "\n";
      } else {
        errs() << "\n<<function: NULL>>\n";
      }
      for (auto I = cgn->begin(), E = cgn->end(); I != E; ++I) {
        Value *cs = I->first;
        if (cs) {
          errs() << *cs << "\t";
          CallGraphNode *calleeNode = I->second;
          Function *callee = calleeNode->getFunction();
          if (callee == nullptr) {
            /// errs() << calleeNode << "\n";
            assert(calleeNode == externCallee);
            Instruction *csInst = cast<Instruction>(cs);
            CallSite CS(csInst);
            FnVect callees(findPossibleCallee(CS.getCalledValue()));
            for (auto *calledFn : callees) {
              errs() << calledFn->getName() << " ";
              cgn->replaceCallEdge(CS, CS, cg[calledFn]);
            }
            errs() << "\n";
          } else {
            /// errs() << callee->getName() << "\n";
          }
        } else {
          /// errs() << "CallSite not exist\n";
        }
      }
    }
    cg.print(errs());

    return false;
  }

  virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }
};

char TestCG::ID = 0;
static RegisterPass<TestCG> X("TestCG", "TestCG", true, true);
}
