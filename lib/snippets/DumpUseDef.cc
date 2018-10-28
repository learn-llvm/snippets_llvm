#define DEBUG_TYPE "TestUser"

#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/TypeBuilder.h"
#include "llvm/IRReader/IRReader.h"

#include "llvm/IR/Verifier.h"

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"

#include "LLDump.hh"

using namespace llvm;

struct DumpUseDef final : public ModulePass {
  static char ID;

  DumpUseDef() : ModulePass(ID) {}

  void dumpUsesInfo(Value &V) {
    if (V.getNumUses() > 0) {
      WITH_COLOR(raw_ostream::YELLOW, errs() << "uses of: ";);
      prettyPrint(&V);
      errs() << "\n";
      for (auto b = V.use_begin(), e = V.use_end(); b != e; ++b) {
        prettyPrint(*b, 0, 0);
      }
    }
  }

  void dumpAllUsesInFunc(Function &F) {
    if (F.getNumUses() > 0) {
      dumpUsesInfo(F);
    }
    for (auto &B : F) {
      dumpUsesInfo(B);
      for (auto &I : B) {
        dumpUsesInfo(I);
      }
    }
  }

  bool runOnModule(Module &M) override {
    for (auto &F : M) {
      if (F.isDeclaration()) {
        continue;
      }
      dumpAllUsesInFunc(F);
    }
    return false;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }
};

char DumpUseDef::ID = 0;
static RegisterPass<DumpUseDef> X("DumpUseDef", "DumpUseDef pass", true, true);
