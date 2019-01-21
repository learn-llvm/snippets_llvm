#define DEBUG_TYPE "DumpConsts"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"

#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include "LLDump.hh"

using namespace llvm;

namespace {

struct DumpConsts final : public ModulePass {
  static char ID;

  DumpConsts() : ModulePass(ID) {}

  void dumpConst(Constant *c, DataLayout const &dl) {
    if (ConstantInt *constantInt = dyn_cast<ConstantInt>(c)) {
      errs() << "ConstantInt: value=" << constantInt->getValue() << "\n";
    } else if (ConstantFP *constantFP = dyn_cast<ConstantFP>(c)) {
      APFloat apFloat = constantFP->getValueAPF();
      errs() << "ConstantFP: value=" << ToString(&apFloat) << "\n";
    } else if (ConstantArray *constantArray = dyn_cast<ConstantArray>(c)) {
      auto valueName = constantArray->getValueName();
      errs() << "ConstantArray: " << valueName->getKey() << ": " << valueName->getValue() << "\n";
    } else if (ConstantPointerNull *constantPointerNull = dyn_cast<ConstantPointerNull>(c)) {
      errs() << "ConstantPointerNull: type=" << ToString(constantPointerNull->getType()) << "\n";
    } else if (ConstantTokenNone *constantTokenNone = dyn_cast<ConstantTokenNone>(c)) {
      errs() << "ConstantTokenNone: operands=" << constantTokenNone->getNumOperands()
             << "\tpointerAlign=" << constantTokenNone->getPointerAlignment(dl) << "\n";
    } else if (UndefValue *undefValue = dyn_cast<UndefValue>(c)) {
      errs() << "UndefValue: pointAlign=" << undefValue->getPointerAlignment(dl) << "\n";
    } else if (ConstantDataArray *constantDataArray = dyn_cast<ConstantDataArray>(c)) {
      auto v = constantDataArray->getValueName();
      errs() << "ConstantDataArray: " << v;
    } else if (ConstantStruct *constantStruct = dyn_cast<ConstantStruct>(c)) {
      auto v = constantStruct->getValueName();
      errs() << "ConstantStruct: " << v;
    } else if (ConstantExpr *constantExpr = dyn_cast<ConstantExpr>(c)) {
      errs() << "ConstantExpr: " << ToString(constantExpr->getType()) << "\n";
    } else if (GlobalValue *globalValue = dyn_cast<GlobalValue>(c)) {
      if (GlobalIFunc *globalIFunc = dyn_cast<GlobalIFunc>(globalValue)) {
        errs() << "GlobalIFunc: " << globalIFunc->getName() << "\n";
      } else if (GlobalAlias *globalAlias = dyn_cast<GlobalAlias>(globalValue)) {
        errs() << "GlobalAlias: " << globalAlias->getName() << "\n";
      } else if (GlobalVariable *globalVariable = dyn_cast<GlobalVariable>(globalValue)) {
        errs() << "GlobalVariable: " << globalVariable->getName() << "\n";
      } else if (Function *function = dyn_cast<Function>(globalValue)) {
        errs() << "Function: " << function->getName() << "\n";
      }
    } else {
      llvm_unreachable("not implemented");
    }
  }

  bool runOnModule(Module &M) override {

    DataLayout const &dl = M.getDataLayout();

    for (Function &F: M) {
      for (BasicBlock &B: F) {
        for (Instruction &I: B) {
          errs() << "INST:" << I << "\n";
          unsigned op_num = I.getNumOperands();
          for (unsigned i = 0; i < op_num; ++i) {
            Value *v = I.getOperand(i);
            if (Constant *constant = dyn_cast<Constant>(v)) {
              errs() << " i=" << i << ", CONST=";
              if (GlobalValue *globalValue = dyn_cast<GlobalValue>(constant)) {
                errs() << globalValue->getName() << "\n";
              } else {
                errs() << *constant << "\n";
                dumpConst(constant, dl);
              }
            }
          }
        }
      }
    }

    return false;
  }

  virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }
};

char DumpConsts::ID = 0;
static RegisterPass<DumpConsts> X("DumpConsts", "DumpConsts pass", true, true);
}  // namespace
