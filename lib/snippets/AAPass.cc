#define DEBUG_TYPE "AAPass"
#include "Version.hh"

#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/TypeBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IRReader/IRReader.h"

#if LLVM_VERSION_CODE >= LLVM_VERSION(3, 5)
#include "llvm/IR/Verifier.h"
#else
#include "llvm/Analysis/Verifier.h"
#endif

#include "llvm/Pass.h"
#include "llvm/PassManager.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/TargetSelect.h"

#include "FnUtils.hh"
#include "Logging.hh"

using namespace llvm;

namespace {

struct AAPass final : public FunctionPass {
  static char ID;
  /// DataLayout *datalayout;

  virtual void print(llvm::raw_ostream &O, const Module *M) const override {}

  AAPass() : FunctionPass(ID) {}

  /// virtual bool doInitialization(Module &M) override {
  ///   datalayout = new DataLayout(&M);
  ///   return false;
  /// }

  void dumpAALocation(AliasAnalysis &AA, AliasAnalysis::Location &L) {
#if LLVM_VERSION_CODE >= LLVM_VERSION(3, 5)
    DataLayout const &DL = getAnalysis<DataLayoutPass>().getDataLayout();
#else
    DataLayout &DL = getAnalysis<DataLayout>();
#endif
    WITH_COLOR(raw_ostream::GREEN, errs() << "ptr:");
    errs() << *L.Ptr << "\n";
    Type *ty = L.Ptr->getType();
    WITH_COLOR(raw_ostream::CYAN, errs() << "actualSize:\t");
    errs() << "s=" << DL.getTypeSizeInBits(ty)
           << "\t(s+7)/8=" << DL.getTypeStoreSize(ty) << "\tptr_s=" << L.Size
           << "\n";
#if LLVM_VERSION_CODE >= LLVM_VERSION(3, 5)
    if (L.AATags) {
      WITH_COLOR(raw_ostream::CYAN, errs() << "tbaa tag: ");
      errs() << *L.AATags.TBAA << "\n";
    }
#else
#include "llvm/Analysis/Verifier.h"
    if (L.TBAATag) {
      WITH_COLOR(raw_ostream::CYAN, errs() << "tbaa tag: ");
      errs() << *L.TBAATag << "\n";
    }
#endif
    WITH_COLOR(raw_ostream::MAGENTA, errs() << "pointstoConstantMemory: ");
    errs() << AA.pointsToConstantMemory(L) << "\n";
    errs() << "\n";
  }

  void dumpAAInfo(AliasAnalysis &AA) {}

  bool runOnFunction(Function &F) override {
    WITH_COLOR(raw_ostream::RED, errs() << "Function: ");
    errs() << F.getName() << "\n";
    /// AliasAnalysis &AA = getAnalysis<AliasAnalysis>(F);
    AliasAnalysis &AA = getAnalysis<AliasAnalysis>();
    for (auto &B : F)
      for (auto &I : B) {
        if (LoadInst *LI = dyn_cast<LoadInst>(&I)) {
          WITH_COLOR(raw_ostream::YELLOW, errs() << "AA Location for");
          errs() << *LI << "\n";
          AliasAnalysis::Location loc = AA.getLocation(LI);
          dumpAALocation(AA, loc);
        }
        if (MemIntrinsic const *memIntrinsic = dyn_cast<MemIntrinsic>(&I)) {
          AliasAnalysis::Location dst =
              AliasAnalysis::getLocationForDest(memIntrinsic);
          WITH_COLOR(raw_ostream::YELLOW, errs() << "dst for MemIntrinsic: ");
          errs() << *memIntrinsic << "\n";
          dumpAALocation(AA, dst);
        }
        if (MemTransferInst const *memTransferInst =
                dyn_cast<MemTransferInst>(&I)) {
          WITH_COLOR(raw_ostream::YELLOW, errs()
                                              << "src for MemTransferInst: ");
          errs() << *memTransferInst << "\n";
          AliasAnalysis::Location src =
              AliasAnalysis::getLocationForSource(memTransferInst);
          dumpAALocation(AA, src);
        }
      }
    return false;
  }

  /// bool runOnModule(Module &M) {
  ///   for (auto &F : M) {
  ///     if (F.isDeclaration()) continue;
  ///     runOnFunction(F);
  ///   }
  ///   return false;
  /// }

  virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<AliasAnalysis>();
#if LLVM_VERSION_CODE >= LLVM_VERSION(3, 5)
    AU.addRequired<DataLayoutPass>();
#else
    AU.addRequired<DataLayout>();
#endif
    AU.setPreservesAll();
  }
};

char AAPass::ID = 0;

static RegisterPass<AAPass> X("AAPass", "AAPass", false, false);
}
