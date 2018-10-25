#define DEBUG_TYPE "count-bb"

#include "llvm/Pass.h"

#include "llvm/IR/Attributes.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/TypeBuilder.h"

#include "llvm/Support/raw_ostream.h"

#include "FnUtils.hh"
#include <vector>

using namespace llvm;

namespace {
static char const *k_atexitCallStr = "setupAtExit";
static char const *bbCounterStr = "bbCounter";

namespace detail {
void create_atexitCall(Function &F, GlobalVariable *BBCounter) {
  Module &M = *F.getParent();
  LLVMContext &ctx = M.getContext();
  auto *entryBB = BasicBlock::Create(ctx, "entry", &F);
  FunctionType *funTy =
      TypeBuilder<int(char *, ...), false>::get(M.getContext());
  auto *printfFn = cast<Function>(M.getOrInsertFunction("printf", funTy));
  auto bbValue = new LoadInst(BBCounter, "bbValue", entryBB);
  Constant *fmt = utils::geti8StrVal(M, "%d Executed\n", "fmt");
  ArrayRef<Value *> printfArgs{fmt, bbValue};
  CallInst::Create(printfFn, printfArgs, "", entryBB);
  ReturnInst::Create(ctx, entryBB);
}
}

struct CountBBPass : public ModulePass {

  static char ID;

  CountBBPass() : ModulePass(ID) {}

  bool runOnModule(Module &M) override;
  bool runOnBasicBlock(BasicBlock &BB, Module &M);

  bool setup(Module &M);
};
}

char CountBBPass::ID = 0;

static RegisterPass<CountBBPass> X("count-bb",
                                   "Counts no. of executed BasicBlocks", false,
                                   false);

/// -------------------------------------------------------------------------
/// ///

bool CountBBPass::setup(Module &M) {
  auto *Main = M.getFunction("main");
  if (Main == nullptr) {
    std::fprintf(stderr, "cannot find main\n");
    std::exit(1);
  }

  /// Type *i64Ty = Type::getInt64Ty(M.getContext());
  Type *i64Ty = TypeBuilder<types::i<64>, false>::get(M.getContext());

  auto bbCounter =
      new GlobalVariable(M, i64Ty, false, GlobalValue::ExternalLinkage,
                         ConstantInt::get(i64Ty, 0), "bbCounter");

  auto *aexitCall = cast<Function>(M.getOrInsertFunction(
      k_atexitCallStr, TypeBuilder<void(void), false>::get(M.getContext())));
  auto *atexitFn = cast<Function>(M.getOrInsertFunction(
      "atexit", TypeBuilder<int(void(void)), false>::get(M.getContext()),
      AttributeSet().addAttribute(M.getContext(), ~0U, Attribute::NoUnwind)));

  Instruction &I = Main->front().front();
  CallInst::Create(atexitFn, aexitCall, "", &I);

  detail::create_atexitCall(*aexitCall, bbCounter);

  return true;
}

bool CountBBPass::runOnBasicBlock(BasicBlock &BB, Module &M) {
  auto *bbCounter = M.getGlobalVariable(bbCounterStr);
  assert(bbCounter && "Error: unable to get BasicBlock counter");

  auto *insertPos = BB.getTerminator();

  auto *oldCounter = new LoadInst(bbCounter, "old.bb.count", insertPos);
  auto *newCounter = BinaryOperator::Create(
      Instruction::Add, oldCounter,
      ConstantInt::get(Type::getInt64Ty(BB.getContext()), 1), "new.bb.count",
      insertPos);
  new StoreInst(newCounter, bbCounter, insertPos);

  return true;
}

bool CountBBPass::runOnModule(Module &M) {
  setup(M);
  for (auto &F : M) {
    if (F.getName() == k_atexitCallStr) continue;
    for (auto &B : F) runOnBasicBlock(B, M);
  }
  return true;
}
