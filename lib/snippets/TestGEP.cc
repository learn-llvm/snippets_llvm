#define DEBUG_TYPE "TestGEP"
#include "llvm/Support/Debug.h"

#include "Version.hh"
#if LLVM_VERSION_CODE >= LLVM_VERSION(3, 3)
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#else
#include "llvm/Function.h"
#include "llvm/BasicBlock.h"
#include "llvm/Instructions.h"
#endif

#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include "Logging.hh"

using namespace llvm;

bool isGEPInst(Instruction &I) { return isa<GetElementPtrInst>(&I); }

struct TestGEP final : public FunctionPass {
  static char ID;
  TestGEP() : FunctionPass(ID) {}
  virtual void print(llvm::raw_ostream &O, const Module *M) const override {
    errs() << __PRETTY_FUNCTION__ << "\n";
  }

  void visitBB(BasicBlock &B, function_ref<bool(Instruction &)> callback) {
    for (auto &I : B) {
      if (callback(I)) {
        errs() << I << " meets callback requirement\n";
      }
    }
  }

  void bb(Function *F){
    for(inst_iterator I = inst_begin(F), E = inst_end(F); I!=E; ++I){
      errs()<<*I<<"\n";
    } 
  }

  bool runOnFunction(Function &F) override {
    for (auto &B : F) {
      errs() << B << "\n";
      visitBB(B, isGEPInst);
      /// for (auto &I : B) {
      ///   if (GetElementPtrInst *gep = dyn_cast<GetElementPtrInst>(&I)) {
      ///     errs() << "\n" << *gep << "\t type -> ";
      ///     logging::printTypeInfo(gep->getType());
      ///     errs() << "pointed: " << *gep->getPointerOperand() << "\t type->
      ///     ";
      ///     logging::printTypeInfo(gep->getPointerOperandType());
      ///     errs() << "addressSpace: " << gep->getAddressSpace() << "\n";
      ///     errs() << "isInBounds: " << gep->isInBounds() << "\n";
      ///     errs() << "numOfIndices: " << gep->getNumIndices() << "\n";
      ///   }
      /// }
    }
    return false;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }
};

char TestGEP::ID = 0;
static RegisterPass<TestGEP> X("TestGEP", "TestGEP pass", true, true);
