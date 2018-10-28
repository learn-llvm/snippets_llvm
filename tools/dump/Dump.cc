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

#include "LLUtils.hh"
#include "Common.hh"
#include "LLDump.hh"

using namespace llvm;

template<typename T>
static std::string ToString(const T *obj) {
  std::string TypeName;
  raw_string_ostream N(TypeName);
  obj->print(N);
  return N.str();
}

struct DumpModulePass : public ModulePass {
  static char ID;

  DumpModulePass() : ModulePass(ID) {}

  void getAnalysisUsage(AnalysisUsage &AU) const override {
  }

  bool runOnBasicBlock(BasicBlock &bb, DataLayout const &dataLayout) {
    WITH_COLOR(raw_ostream::CYAN,
               errs() << "---> BB (in " << bb.getParent()->getName() << "): " << ppName(bb.getName()) << "\n";);
    for (auto &inst : bb) {
      errs() << CLASS_NAME(&inst) << inst << "\n";
      if (auto *allocaInst = dyn_cast<AllocaInst>(&inst)) {
        auto *allocType = allocaInst->getAllocatedType();
        errs() << CLASS_NAME(allocaInst) << " type: " << ToString(allocType) << " allocSize="
               << dataLayout.getTypeSizeInBits(allocType) << " bits\n";
      } else if (auto *gep = dyn_cast<GetElementPtrInst>(&inst)) {
        errs() << CLASS_NAME(gep) << " type: " << ToString(gep->getType()) << "\n";
        errs() << "  pointer operand: " << ToString(gep->getPointerOperand()) << "\n";
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
    Value *calledValue = callInst->getCalledValue();
    if (auto *ia = dyn_cast<InlineAsm>(calledValue)) {
      _dump_InlineASM(ia);
    }
    Function *callee = callInst->getCalledFunction();
    if (callee != nullptr) {
      errs() << "callee: " << callee->getName() << "\tisIntrinsic: " << callee->isIntrinsic() << "\n";
    }
  }

  void _dump_InlineASM(InlineAsm *ia) {
    errs() << "\ninline asm: " << *ia << "\n";
    errs() << "asm string: ";
    errs().write_escaped(ia->getAsmString());
    errs() << "\nconstraint string: " << ia->getConstraintString()
           << "\n";
    errs() << "hasSideEffects: " << ia->hasSideEffects() << "\n";
    errs() << "isAlignStack: " << ia->isAlignStack() << "\n";
    errs() << "dialect: "
           << (ia->getDialect() == InlineAsm::AD_ATT ? "att"
                                                     : "intel")
           << "\n";
    errs() << "Type: " << ia->getType();
    errs() << "\n";
    errs() << "split asm string...\n";
    SmallVector<StringRef, 4> asmPieces;
  }

  void _dumpFnTy(Function &F) {
    FunctionType *fnTy = F.getFunctionType();
    printTypeInfo(fnTy);
    Type *retTy = fnTy->getReturnType();
    errs() << " retType: ";
    printTypeInfo(retTy);
    errs() << "ParameterTypes (" << fnTy->getNumParams() << "):\n";
    unsigned i = 0;
    for (auto *ty : fnTy->params()) {
      i++;
      errs() << " " << i << ") ";
      printTypeInfo(ty);
    }
  }

  void _dump_PHINode(PHINode *phi) {
    errs() << CLASS_NAME(phi) << " incoming: " << phi->getNumIncomingValues() << "\n";
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
    auto &gvarList = M.getGlobalList();
    if (gvarList.empty()) {
      errs() << " empty\n";
    } else {
      errs() << "\n";
      for (auto &gVar: gvarList) {
        errs() << gVar << "\ttype:" << ToString(gVar.getType()) << "\n";
        dumpGVInfo(gVar);
      }
    }
    WITH_COLOR(raw_ostream::RED, errs() << "\n===> global alias:";);
    auto &aliasList = M.getAliasList();
    if (aliasList.empty()) {
      errs() << " empty\n";
    } else {
      for (auto &gAlias: aliasList) {
        errs() << gAlias << " base:" << *gAlias.getBaseObject() << "\n";
        dumpGVInfo(gAlias);
      }
    }
    WITH_COLOR(raw_ostream::RED, errs() << "\n===> ifuncs:";);
    auto &ifuncs = M.getIFuncList();
    if (ifuncs.empty()) {
      errs() << " empty\n";
    } else {
      for (auto &ifunc: ifuncs) {
        errs() << ifunc << "\n";
        dumpGVInfo(ifunc);
      }
    }
  }

  void printNMetadata(Module &M) {
    WITH_COLOR(raw_ostream::MAGENTA, errs() << "\n===> NMetadate:";);
    for (auto &nmd:M.getNamedMDList()) {
      errs() << ToString(&nmd);
      for (unsigned i = 0, e = nmd.getNumOperands(); i != e; ++i) {
        auto *Op = nmd.getOperand(i);
        if (auto *mdNode = dyn_cast<MDNode>(Op)) {
          errs() << "  Has MDNode operand:  " << *mdNode;
          for (auto UI = mdNode->op_begin(), UE = mdNode->op_end(); UI != UE; ++UI) {
            errs() << "   the operand has a user:\n    ";
            errs() << **UI << "\n";
          }
        }
      }
    }
  }

  bool runOnFunc(Function &F, DataLayout const &layout) {
    WITH_COLOR(raw_ostream::RED, errs() << "\n===> FUNC: " << F.getName() << "\n";);
    _dumpFnTy(F);
    dumpGVInfo(F);
    for (auto &B: F) {
      runOnBasicBlock(B, layout);
    }
    return false;
  }

  bool runOnModule(Module &M) override {
    auto &layout = M.getDataLayout();
    for (auto &F: M) {
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
