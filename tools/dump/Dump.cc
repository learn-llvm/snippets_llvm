#include <llvm/IR/IntrinsicInst.h>
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/IR/CFG.h"

#include "LLDump.hh"
#include "LLUtils.hh"

using namespace llvm;

struct DumpModulePass : public ModulePass {
  static char ID;

  DumpModulePass() : ModulePass(ID) {}

  void getAnalysisUsage(AnalysisUsage &AU) const override {}

  void printInstArgType(Instruction &I) {
    errs() << "\nInstruction: " << I << "\n";
    Type *instTy = I.getType();
    if (instTy->isPointerTy()) {
      errs() << "POINT ";
      instTy = dyn_cast<PointerType>(instTy)->getNonOpaquePointerElementType();
    }
    getTypeStr(instTy);
    for (auto &arg : I.operands()) {
      Type *ty = arg->getType();
      if (ty->isPointerTy()) {
        errs() << "POINT ";
        ty = dyn_cast<PointerType>(ty)->getNonOpaquePointerElementType();
      }
      getTypeStr(ty);
    }
  }

  bool runOnBasicBlock(BasicBlock &B, DataLayout const &dataLayout) {
    WITH_COLOR(raw_ostream::CYAN,
               errs() << "---> BB (in " << B.getParent()->getName()
                      << "): " << ppName(B.getName()) << "\n";);
    for (auto &inst : B) {
      errs() << "INST: " << inst << "\n";
      if (auto *allocaInst = dyn_cast<AllocaInst>(&inst)) {
        auto *allocType = allocaInst->getAllocatedType();
        errs() << "AllocaInst type: " << ToString(allocType)
               << " allocSize=" << dataLayout.getTypeSizeInBits(allocType)
               << " bits\n";
      } else if (auto *gep = dyn_cast<GetElementPtrInst>(&inst)) {
        errs() << getValueStr(gep) << " type: " << ToString(gep->getType())
               << "\n";
        errs() << "  pointer operand: " << ToString(gep->getPointerOperand())
               << "\n";
        errs() << "  Indices: ";
        for (auto Idx = gep->idx_begin(), IdxE = gep->idx_end(); Idx != IdxE;
             ++Idx) {
          errs() << "[" << ToString(Idx->get()) << "] ";
        }
        errs() << "\n";
      } else if (auto *phi = dyn_cast<PHINode>(&inst)) {
        _dump_PHINode(phi);
      } else if (auto *CI = dyn_cast<CallInst>(&inst)) {
        _dump_CallInst(CI);
      }
    }
    return false;
  }

  void _dump_CallInst(CallInst *callInst) {
    Value *calledValue = callInst->getCalledFunction();
    if (auto *ia = dyn_cast<InlineAsm>(calledValue)) {
      _dump_InlineASM(ia);
    }
    Function *callee = callInst->getCalledFunction();
    if (callee != nullptr) {
      errs() << "callee: " << callee->getName() << "\t";
      if (auto *ii = dyn_cast<IntrinsicInst>(callInst)) {
        if (auto *cfpi = dyn_cast<ConstrainedFPIntrinsic>(ii)) {
          errs() << "constrainedFPIntrinsic"
                 << "\n";
        } else if (auto *dbgii = dyn_cast<DbgInfoIntrinsic>(ii)) {
          errs() << "DbgInfo\n";
        } else if (auto *vci = dyn_cast<VACopyInst>(ii)) {
          errs() << "va_copy\n";
        } else if (auto *vei = dyn_cast<VAEndInst>(ii)) {
          errs() << "va_end\n";
        } else if (auto *vsi = dyn_cast<VAStartInst>(ii)) {
          errs() << "va_start\n";
        } else if (auto *memset = dyn_cast<MemSetInst>(ii)) {
          errs() << "memset\n";
        } else if (auto *memcpy = dyn_cast<MemCpyInst>(ii)) {
          errs() << "memcpy\n";
        } else if (auto *memmove = dyn_cast<MemMoveInst>(ii)) {
          errs() << "memmove\n";
        } else if (auto *atomic_memcpy = dyn_cast<AtomicMemCpyInst>(ii)) {
          errs() << "automic_memcpy\n";
        } else if (auto *atomic_memmove = dyn_cast<AtomicMemMoveInst>(ii)) {
          errs() << "automic_memmove\n";
        } else if (auto *any_memcpy = dyn_cast<AnyMemCpyInst>(ii)) {
          errs() << "any_memcpy\n";
        } else if (auto *any_memmove = dyn_cast<AnyMemMoveInst>(ii)) {
          errs() << "any_memmove\n";
        } else {
          errs() << "Untracked IntrinsicInst\n";
        }
      } else {
        errs() << "\n";
      }
    }
  }

  void _dump_InlineASM(InlineAsm *ia) {
    errs() << "\ninline asm: " << *ia << "\n";
    errs() << "asm string: ";
    errs().write_escaped(ia->getAsmString());
    errs() << "\nconstraint string: " << ia->getConstraintString() << "\n";
    errs() << "hasSideEffects: " << ia->hasSideEffects() << "\n";
    errs() << "isAlignStack: " << ia->isAlignStack() << "\n";
    errs() << "dialect: "
           << (ia->getDialect() == InlineAsm::AD_ATT ? "att" : "intel") << "\n";
    errs() << "Type: " << ia->getType();
    errs() << "\n";
    errs() << "split asm string...\n";
    SmallVector<StringRef, 4> asmPieces;
  }

  void _dumpFnTy(Function &F) {
    FunctionType *fnTy = F.getFunctionType();
    getTypeStr(fnTy);
    Type *retTy = fnTy->getReturnType();
    errs() << " retType: ";
    getTypeStr(retTy);
    errs() << "ParameterTypes (" << fnTy->getNumParams() << "):\n";
    unsigned i = 0;
    for (auto *ty : fnTy->params()) {
      i++;
      errs() << " " << i << ") ";
      getTypeStr(ty);
    }
  }

  void _dump_PHINode(PHINode *phi) {
    errs() << getValueStr(phi) << " incoming: " << phi->getNumIncomingValues()
           << "\n";
    BasicBlock *B = phi->getParent();
    for (auto pred = pred_begin(B); pred != pred_end(B); ++pred) {
      BasicBlock *predBB = *pred;
      int index = phi->getBasicBlockIndex(predBB);
      errs() << "\tindex of " << predBB->getName() << " is " << index << "\n";
    }
  }

  // global values
  void printGlobalValues(Module &M) {
    WITH_COLOR(raw_ostream::RED, errs() << "\n===> global variables:";);
    WITH_COLOR(raw_ostream::RED, errs() << "\n===> global alias:";);
    WITH_COLOR(raw_ostream::RED, errs() << "\n===> ifuncs:";);
  }

  void printNMetadata(Module &M) {
    WITH_COLOR(raw_ostream::MAGENTA, errs() << "\n===> NMetadate:";);
  }

  bool runOnFunc(Function &F, DataLayout const &layout) {
    WITH_COLOR(raw_ostream::RED,
               errs() << "\n===> FUNC: " << F.getName() << "\n";);
    _dumpFnTy(F);
    dumpGVInfo(F);
    for (auto &B : F) {
      runOnBasicBlock(B, layout);
    }
    return false;
  }

  bool runOnModule(Module &M) override {
    auto &layout = M.getDataLayout();
    for (auto &F : M) {
      runOnFunc(F, layout);
    }
    printGlobalValues(M);
    printNMetadata(M);
    return false;
  }
};

char DumpModulePass::ID = 0;

int main(int argc, char **argv) {
  if (argc < 2) {
    errs() << "Usage: " << argv[0] << " <IR file>\n";
    std::exit(1);
  }

  SMDiagnostic Err;
  LLVMContext ctx;
  std::unique_ptr<Module> Mod(parseIRFile(argv[1], Err, ctx));

  if (!Mod) {
    Err.print(argv[0], errs());
    std::exit(1);
  }

  legacy::PassManager PM;
  PM.add(new DumpModulePass());
  PM.run(*Mod);

  return 0;
}
