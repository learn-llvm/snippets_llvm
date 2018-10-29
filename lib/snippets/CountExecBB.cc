#define DEBUG_TYPE "count-bb"

#include "llvm/Pass.h"

#include "llvm/IR/Attributes.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/TypeBuilder.h"

#include "llvm/Support/raw_ostream.h"

#include "LLUtils.hh"
#include "LLDump.hh"

using namespace llvm;

namespace {
static char const *k_atexitCallStr = "setupAtExit";
static char const *bbCounterStr = "bbCounter";

void create_atexitCall(Function &F, GlobalVariable *BBCounter) {
  auto *M = F.getParent();
  LLVMContext &ctx = M->getContext();
  auto *entryBB = BasicBlock::Create(ctx, "entry", &F);
  FunctionType *funcTy = TypeBuilder<int(char *, ...), false>::get(ctx);
  auto *printfFn = cast<Function>(M->getOrInsertFunction("printf", funcTy));
  auto bbValue = new LoadInst(BBCounter, "bbValue", entryBB);
  Constant *fmt = utils::geti8StrVal(*M, "%d BB(s) Executed\n", "fmt");
  ArrayRef<Value *> printfArgs{fmt, bbValue};
  CallInst::Create(printfFn, printfArgs, "", entryBB);
  ReturnInst::Create(ctx, entryBB);
}

struct CountBBPass : public ModulePass {
  static char ID;

  CountBBPass() : ModulePass(ID) {}

  bool runOnModule(Module &M) override;

  bool runOnBasicBlock(BasicBlock &BB, Module &M);

  bool setup(Module &M);
};
}  // namespace

char CountBBPass::ID = 0;

static RegisterPass<CountBBPass> X("count-bb",
                                   "Counts no. of executed BasicBlocks", false,
                                   false);

bool CountBBPass::setup(Module &M) {

  auto *Main = M.getFunction("main");
  auto &ctx = M.getContext();
  if (Main == nullptr) {
    std::fprintf(stderr, "cannot find main\n");
    std::exit(1);
  }

  Type *i64Ty = TypeBuilder<types::i<64>, false>::get(ctx);

  auto bbCounter = new GlobalVariable(M, i64Ty, false, GlobalValue::ExternalLinkage,
                                      ConstantInt::get(i64Ty, 0), "bbCounter");

  // create a call to the bb tracking callback function
  auto *aexitCall = cast<Function>(M.getOrInsertFunction(k_atexitCallStr, TypeBuilder<void(void), false>::get(ctx)));

  // set the attribute of the atexit
  auto atexit_attr = AttributeList().addAttribute(ctx, ~0U, Attribute::NoUnwind);
  // signature of atexitFn
  auto *atexitFn =
      cast<Function>(M.getOrInsertFunction("atexit", TypeBuilder<int(void(void)), false>::get(ctx), atexit_attr));

  Instruction &I = Main->front().front();
  CallInst::Create(atexitFn, aexitCall, "", &I);

  create_atexitCall(*aexitCall, bbCounter);

  return true;
}

bool CountBBPass::runOnBasicBlock(BasicBlock &BB, Module &M) {

  // get the variable according to variable name, the value is default to 0
  auto *bbCounter = M.getGlobalVariable(bbCounterStr);
  assert(bbCounter && "Error: unable to get BasicBlock counter");

  // insert at the end of the current bb (before the terminator)
  auto *insertPos = BB.getTerminator();

  // load the value of bbCounter
  auto *oldCounter = new LoadInst(bbCounter, "old.bb.count", insertPos);
  // inc 1 of the counter and store
  auto *newCounter = BinaryOperator::Create(
      Instruction::Add, oldCounter,
      ConstantInt::get(Type::getInt64Ty(BB.getContext()), 1), "new.bb.count",
      insertPos);
  // insert store inst
  new StoreInst(newCounter, bbCounter, insertPos);

  return true;
}

bool CountBBPass::runOnModule(Module &M) {

  setup(M);
  for (auto &F : M) {
    // skip analysis on the instrumented function
    if (F.getName() == k_atexitCallStr) continue;
    for (auto &B : F) {
      runOnBasicBlock(B, M);
    }
  }
  return true;
}
