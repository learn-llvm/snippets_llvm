#include "Version.hh"

#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/IRBuilder.h"

#include "llvm/IR/DataLayout.h"
#include "llvm/Pass.h"
#include "llvm/PassManager.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/CodeGen/IntrinsicLowering.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/PassManager.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Assembly/PrintModulePass.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TargetLowering.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetCallingConv.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/Host.h"

#include <set>

using namespace llvm;

static cl::opt<std::string> myOutputFileName(
    "o", cl::desc("override output filename"), cl::value_desc("filename"));

static cl::opt<std::string> myInputFileName(cl::Positional,
                                            cl::desc("<input filename>"),
                                            cl::init("-"),
                                            cl::value_desc("filename"));

///--------------------------------------------------------------------------------------///

class PhiCleanerPass : public llvm::FunctionPass {
  static char ID;

 public:
  PhiCleanerPass() : llvm::FunctionPass(ID) {}

  virtual bool runOnFunction(llvm::Function &f);
};

class DivCheckPass : public llvm::ModulePass {
  static char ID;

 public:
  DivCheckPass() : ModulePass(ID) {}
  virtual bool runOnModule(llvm::Module &M);
};

char PhiCleanerPass::ID = 0;

bool PhiCleanerPass::runOnFunction(Function &f) {
  bool changed = false;

  for (Function::iterator b = f.begin(), be = f.end(); b != be; ++b) {
    BasicBlock::iterator it = b->begin();

    if (it->getOpcode() == Instruction::PHI) {
      PHINode *reference = cast<PHINode>(it);

      std::set<Value *> phis;
      phis.insert(reference);

      unsigned numBlocks = reference->getNumIncomingValues();
      for (++it; isa<PHINode>(*it); ++it) {
        PHINode *pi = cast<PHINode>(it);

        assert(numBlocks == pi->getNumIncomingValues());

        // see if it is out of order
        unsigned i;
        for (i = 0; i < numBlocks; i++)
          if (pi->getIncomingBlock(i) != reference->getIncomingBlock(i)) break;

        if (i != numBlocks) {
          std::vector<Value *> values;
          values.reserve(numBlocks);
          for (unsigned i = 0; i < numBlocks; i++)
            values[i] =
                pi->getIncomingValueForBlock(reference->getIncomingBlock(i));
          for (unsigned i = 0; i < numBlocks; i++) {
            pi->setIncomingBlock(i, reference->getIncomingBlock(i));
            pi->setIncomingValue(i, values[i]);
          }
          changed = true;
        }

        // see if it uses any previously defined phi nodes
        for (i = 0; i < numBlocks; i++) {
          Value *value = pi->getIncomingValue(i);

          if (phis.find(value) != phis.end()) {
            // fix by making a "move" at the end of the incoming block
            // to a new temporary, which is thus known not to be a phi
            // result. we could be somewhat more efficient about this
            // by sharing temps and by reordering phi instructions so
            // this isn't completely necessary, but in the end this is
            // just a pathological case which does not occur very
            // often.
            Instruction *tmp = new BitCastInst(
                value, value->getType(), value->getName() + ".phiclean",
                pi->getIncomingBlock(i)->getTerminator());
            pi->setIncomingValue(i, tmp);
          }

          changed = true;
        }

        phis.insert(pi);
      }
    }
  }

  return changed;
}

///--------------------------------------------------------------------------------------///

class RaiseAsmPass : public llvm::ModulePass {
  static char ID;

  const llvm::TargetLowering *TLI;

  llvm::Function *getIntrinsic(llvm::Module &M, unsigned IID, llvm::Type **Tys,
                               unsigned NumTys);
  llvm::Function *getIntrinsic(llvm::Module &M, unsigned IID, llvm::Type *Ty0) {
    return getIntrinsic(M, IID, &Ty0, 1);
  }

  bool runOnInstruction(llvm::Module &M, llvm::Instruction *I);

 public:
  RaiseAsmPass() : llvm::ModulePass(ID), TLI(0) {}

  virtual bool runOnModule(llvm::Module &M);
};

char RaiseAsmPass::ID = 0;

Function *RaiseAsmPass::getIntrinsic(llvm::Module &M, unsigned IID, Type **Tys,
                                     unsigned NumTys) {
#if LLVM_VERSION_CODE >= LLVM_VERSION(3, 0)
  return Intrinsic::getDeclaration(&M, (llvm::Intrinsic::ID)IID,
                                   llvm::ArrayRef<llvm::Type *>(Tys, NumTys));
#else
  return Intrinsic::getDeclaration(&M, (llvm::Intrinsic::ID)IID, Tys, NumTys);
#endif
}

// FIXME: This should just be implemented as a patch to
// X86TargetAsmInfo.cpp, then everyone will benefit.
bool RaiseAsmPass::runOnInstruction(Module &M, Instruction *I) {
  if (CallInst *ci = dyn_cast<CallInst>(I)) {
    if (InlineAsm *ia = dyn_cast<InlineAsm>(ci->getCalledValue())) {
      (void)ia;
      return TLI && TLI->ExpandInlineAsm(ci);
    }
  }

  return false;
}

bool RaiseAsmPass::runOnModule(Module &M) {
  bool changed = false;

  std::string Err;
#if LLVM_VERSION_CODE >= LLVM_VERSION(3, 1)
  std::string HostTriple = llvm::sys::getDefaultTargetTriple();
#else
  std::string HostTriple = llvm::sys::getHostTriple();
#endif
  errs() << "host triple: " << HostTriple << "\n";
  const Target *NativeTarget = TargetRegistry::lookupTarget(HostTriple, Err);
  if (NativeTarget == 0) {
    llvm::errs() << "Warning: unable to select native target: " << Err << "\n";
    TLI = 0;
  } else {
#if LLVM_VERSION_CODE >= LLVM_VERSION(3, 1)
    TargetMachine *TM =
        NativeTarget->createTargetMachine(HostTriple, "", "", TargetOptions());
#elif LLVM_VERSION_CODE >= LLVM_VERSION(3, 0)
    TargetMachine *TM = NativeTarget->createTargetMachine(HostTriple, "", "");
#else
    TargetMachine *TM = NativeTarget->createTargetMachine(HostTriple, "");
#endif
    TLI = TM->getTargetLowering();
  }

  for (Module::iterator fi = M.begin(), fe = M.end(); fi != fe; ++fi) {
    for (Function::iterator bi = fi->begin(), be = fi->end(); bi != be; ++bi) {
      for (BasicBlock::iterator ii = bi->begin(), ie = bi->end(); ii != ie;) {
        Instruction *i = ii;
        ++ii;
        changed |= runOnInstruction(M, i);
      }
    }
  }

  return changed;
}

///--------------------------------------------------------------------------------------///

class LowerSwitchPass : public llvm::FunctionPass {
 public:
  static char ID;
  LowerSwitchPass() : FunctionPass(ID) {}

  virtual bool runOnFunction(llvm::Function &F);

  struct SwitchCase {
    llvm::Constant *value;
    llvm::BasicBlock *block;

    SwitchCase() : value(0), block(0) {}
    SwitchCase(llvm::Constant *v, llvm::BasicBlock *b) : value(v), block(b) {}
  };

  typedef std::vector<SwitchCase> CaseVector;
  typedef std::vector<SwitchCase>::iterator CaseItr;

 private:
  void processSwitchInst(llvm::SwitchInst *SI);
  void switchConvert(CaseItr begin, CaseItr end, llvm::Value *value,
                     llvm::BasicBlock *origBlock,
                     llvm::BasicBlock *defaultBlock);
};

char LowerSwitchPass::ID = 0;

// The comparison function for sorting the switch case values in the vector.
struct SwitchCaseCmp {
  bool operator()(const LowerSwitchPass::SwitchCase &C1,
                  const LowerSwitchPass::SwitchCase &C2) {

    const ConstantInt *CI1 = cast<const ConstantInt>(C1.value);
    const ConstantInt *CI2 = cast<const ConstantInt>(C2.value);
    return CI1->getValue().slt(CI2->getValue());
  }
};

bool LowerSwitchPass::runOnFunction(Function &F) {
  bool changed = false;

  for (auto &B : F) {
    if (SwitchInst *SI = dyn_cast<SwitchInst>(B.getTerminator())) {
      changed = true;
      processSwitchInst(SI);
    }
  }

  return changed;
}

// switchConvert - Convert the switch statement into a linear scan
// through all the case values
void LowerSwitchPass::switchConvert(CaseItr begin, CaseItr end, Value *value,
                                    BasicBlock *origBlock,
                                    BasicBlock *defaultBlock) {
  BasicBlock *curHead = defaultBlock;
  Function *F = origBlock->getParent();

  // iterate through all the cases, creating a new BasicBlock for each
  for (CaseItr it = begin; it < end; ++it) {
    BasicBlock *newBlock = BasicBlock::Create(getGlobalContext(), "NodeBlock");
    Function::iterator FI = origBlock;
    /// add newBlock after origBlock
    F->getBasicBlockList().insert(++FI, newBlock);

    /// insert compInst at end of newBlock
    ICmpInst *cmpInst = new ICmpInst(*newBlock, ICmpInst::ICMP_EQ, value,
                                     it->value, "case.cmp");
    /// conditional branch; true for switch-branch, false for next case branch;
    /// current is newBlock
    BranchInst::Create(it->block, curHead, cmpInst, newBlock);

    // If there were any PHI nodes in this successor, rewrite one entry
    // from origBlock to come from newBlock.
    for (BasicBlock::iterator bi = it->block->begin(); isa<PHINode>(bi); ++bi) {
      PHINode *PN = cast<PHINode>(bi);

      int blockIndex = PN->getBasicBlockIndex(origBlock);
      assert(blockIndex != -1 && "Switch didn't go to this successor??");
      PN->setIncomingBlock((unsigned)blockIndex, newBlock);
    }

    curHead = newBlock;
  }

  // Branch to our shiny new if-then stuff, unconditional branch
  BranchInst::Create(curHead, origBlock);
}

// processSwitchInst - Replace the specified switch instruction with a sequence
// of chained if-then instructions.
//
void LowerSwitchPass::processSwitchInst(SwitchInst *SI) {
  BasicBlock *origBlock = SI->getParent();
  BasicBlock *defaultBlock = SI->getDefaultDest();
  Function *F = origBlock->getParent();
  Value *switchValue = SI->getCondition();

  // Create a new, empty default block so that the new hierarchy of
  // if-then statements go to this and the PHI nodes are happy.
  BasicBlock *newDefault = BasicBlock::Create(getGlobalContext(), "newDefault");

  F->getBasicBlockList().insert(defaultBlock, newDefault);
  BranchInst::Create(defaultBlock, newDefault);

  // If there is an entry in any PHI nodes for the default edge, make sure
  // to update them as well.
  for (BasicBlock::iterator I = defaultBlock->begin(); isa<PHINode>(I); ++I) {
    PHINode *PN = cast<PHINode>(I);
    int BlockIdx = PN->getBasicBlockIndex(origBlock);
    assert(BlockIdx != -1 && "Switch didn't go to this successor??");
    PN->setIncomingBlock((unsigned)BlockIdx, newDefault);
  }

  CaseVector cases;

#if LLVM_VERSION_CODE >= LLVM_VERSION(3, 1)
  for (SwitchInst::CaseIt i = SI->case_begin(), e = SI->case_end(); i != e; ++i)
    cases.push_back(SwitchCase(i.getCaseValue(), i.getCaseSuccessor()));
#else
  for (unsigned i = 1; i < SI->getNumSuccessors(); ++i)
    cases.push_back(SwitchCase(SI->getSuccessorValue(i), SI->getSuccessor(i)));
#endif

  // reverse cases, as switchConvert constructs a chain of
  //   basic blocks by appending to the front. if we reverse,
  //   the if comparisons will happen in the same order
  //   as the cases appear in the switch
  std::reverse(cases.begin(), cases.end());

  switchConvert(cases.begin(), cases.end(), switchValue, origBlock, newDefault);

  // We are now done with the switch instruction, so delete it
  origBlock->getInstList().erase(SI);
}

///--------------------------------------------------------------------------------------///

class IntrinsicCleanerPass : public ModulePass {
  static char ID;
  const llvm::DataLayout &dataLayout_;
  llvm::IntrinsicLowering *IL_;
  bool lowerIntrinsics_;

  bool runOnBasicBlock(llvm::BasicBlock &b, llvm::Module &M);

 public:
  IntrinsicCleanerPass(const llvm::DataLayout &TD, bool LI = true)
      : llvm::ModulePass(ID),
        dataLayout_(TD),
        IL_(new llvm::IntrinsicLowering(TD)),
        lowerIntrinsics_(LI) {}
  ~IntrinsicCleanerPass() { delete IL_; }

  void getAnalysisUsage(AnalysisUsage &AU) const override {}

  virtual bool runOnModule(llvm::Module &M) override;
};

char IntrinsicCleanerPass::ID;

bool IntrinsicCleanerPass::runOnModule(Module &M) {
  bool dirty = false;
  static unsigned COUNT = 0;
  for (auto &F : M) {
    for (auto &B : F) {
      dirty |= runOnBasicBlock(B, M);
      errs() << "counter: " << COUNT++ << "\n";
    }
  }
  if (Function *Declare = M.getFunction("llvm.trap"))
    Declare->eraseFromParent();
  return dirty;
}

bool IntrinsicCleanerPass::runOnBasicBlock(BasicBlock &b, Module &M) {
  bool dirty = false;

  unsigned WordSize = dataLayout_.getPointerSizeInBits() / 8;
  assert(sizeof(int *) == WordSize);
  for (BasicBlock::iterator i = b.begin(), ie = b.end(); i != ie;) {
    IntrinsicInst *ii = dyn_cast<IntrinsicInst>(&*i);
    // increment now since LowerIntrinsic deletion makes iterator invalid.
    ++i;
    if (ii) {
      errs() << *ii << "\n";
      switch (ii->getIntrinsicID()) {
        case Intrinsic::vastart:
        case Intrinsic::vaend:
          break;

        // Lower vacopy so that object resolution etc is handled by
        // normal instructions.
        //
        // FIXME: This is much more target dependent than just the word size,
        // however this works for x86-32 and x86-64.
        case Intrinsic::vacopy
            : {  // (dst, src) -> *((i8**) dst) = *((i8**) src)
          Value *dst = ii->getArgOperand(0);
          Value *src = ii->getArgOperand(1);

          if (WordSize == 4) {
            Type *i8pp = PointerType::getUnqual(
                PointerType::getUnqual(Type::getInt8Ty(getGlobalContext())));
            Value *castedDst =
                CastInst::CreatePointerCast(dst, i8pp, "vacopy.cast.dst", ii);
            Value *castedSrc =
                CastInst::CreatePointerCast(src, i8pp, "vacopy.cast.src", ii);
            Value *load = new LoadInst(castedSrc, "vacopy.read", ii);
            new StoreInst(load, castedDst, false, ii);
          } else {
            assert(WordSize == 8 && "Invalid word size!");
            Type *i64p =
                PointerType::getUnqual(Type::getInt64Ty(getGlobalContext()));
            Value *pDst =
                CastInst::CreatePointerCast(dst, i64p, "vacopy.cast.dst", ii);
            Value *pSrc =
                CastInst::CreatePointerCast(src, i64p, "vacopy.cast.src", ii);
            Value *val = new LoadInst(pSrc, std::string(), ii);
            new StoreInst(val, pDst, ii);
            Value *off =
                ConstantInt::get(Type::getInt64Ty(getGlobalContext()), 1);
            pDst = GetElementPtrInst::Create(pDst, off, std::string(), ii);
            pSrc = GetElementPtrInst::Create(pSrc, off, std::string(), ii);
            val = new LoadInst(pSrc, std::string(), ii);
            new StoreInst(val, pDst, ii);
            pDst = GetElementPtrInst::Create(pDst, off, std::string(), ii);
            pSrc = GetElementPtrInst::Create(pSrc, off, std::string(), ii);
            val = new LoadInst(pSrc, std::string(), ii);
            new StoreInst(val, pDst, ii);
          }
          ii->removeFromParent();
          delete ii;
          break;
        }

        /// any need to add more intrinsics transformations?
        //  intrinsic return value: {value, carry}
        case Intrinsic::uadd_with_overflow:
        case Intrinsic::umul_with_overflow: {
          IRBuilder<> builder(ii->getParent(), ii);

          Value *op1 = ii->getArgOperand(0);
          Value *op2 = ii->getArgOperand(1);

          Value *result = 0;
          if (ii->getIntrinsicID() == Intrinsic::uadd_with_overflow)
            result = builder.CreateAdd(op1, op2);
          else
            result = builder.CreateMul(op1, op2);

          Value *overflow = builder.CreateICmpULT(result, op1);

          Value *resultStruct = builder.CreateInsertValue(
              UndefValue::get(ii->getType()), result, 0);
          resultStruct = builder.CreateInsertValue(resultStruct, overflow, 1);

          ii->replaceAllUsesWith(resultStruct);
          ii->removeFromParent();
          delete ii;
          dirty = true;
          break;
        }

        case Intrinsic::dbg_value:
        case Intrinsic::dbg_declare:
          // Remove these regardless of lower intrinsics flag. This can
          // be removed once IntrinsicLowering is fixed to not have bad
          // caches.
          ii->eraseFromParent();
          dirty = true;
          break;

        case Intrinsic::trap: {
          // Intrisic instruction "llvm.trap" found. Directly lower it to
          // a call of the abort() function.
          Function *F = cast<Function>(M.getOrInsertFunction(
              "abort", Type::getVoidTy(getGlobalContext()), NULL));
          F->setDoesNotReturn();
          F->setDoesNotThrow();

          CallInst::Create(F, Twine(), ii);
          new UnreachableInst(getGlobalContext(), ii);

          ii->eraseFromParent();

          dirty = true;
          break;
        }

        default:
          if (lowerIntrinsics_) IL_->LowerIntrinsicCall(ii);
          dirty = true;
          break;
      }
    }
  }

  /// errs() << "NOW:" << M << '\n';

  return dirty;
}

int main(int argc, char **argv) {
  cl::ParseCommandLineOptions(argc, argv, "test pass");
  /// char const *outfileStr = myOutputFileName.data();
  /// if (!sys::fs::is_regular_file(outfileStr) ||
  /// sys::fs::can_write(outfileStr)) {
  ///   errs() << outfileStr << " is not regular file or cannot be written\n";
  ///   return 1;
  /// }

  if (myOutputFileName.empty()) {
    myOutputFileName = "-";
  }

  errs() << "input: " << myInputFileName << "\noutput:" << myOutputFileName
         << "\n";

  InitializeNativeTarget();

  SMDiagnostic Err;
  auto &context = getGlobalContext();
  std::unique_ptr<Module> mod(ParseIRFile(myInputFileName, Err, context));

  PassManager PM;
  DataLayout dataLayout(mod.get());
  /// PM.add(new IntrinsicCleanerPass(dataLayout));
  /// PM.add(new LowerSwitchPass());
  /// PM.add(new RaiseAsmPass());
  PM.add(new PhiCleanerPass());

  std::string ErrorInfo;

  raw_fd_ostream OS(myOutputFileName.data(), ErrorInfo, sys::fs::F_Binary);

  if (!ErrorInfo.empty()) {
    errs() << ErrorInfo << "\n";
    return 1;
  }

  PM.add(createPrintModulePass(&OS, false, "HELLO"));

  cl::PrintOptionValues();

  PM.run(*mod);

  return 0;
}
