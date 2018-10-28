#define DEBUG_TYPE "ADD_MTRACE"

#include "llvm/IR/Attributes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"

#include "llvm/Pass.h"

using namespace llvm;

namespace {
class AddMtrace : public ModulePass {
  Constant *f_mtrace, *f_muntrace;

 public:
  static char ID;

  AddMtrace() : ModulePass(ID) {}

  virtual bool runOnModule(Module &M) override {
    // add mtrace/muntrace declaration
    auto attrs =
        AttributeList().addAttribute(M.getContext(), ~0U, Attribute::NoUnwind);
    f_mtrace = M.getOrInsertFunction("mtrace", attrs,
                                     Type::getVoidTy(M.getContext()), nullptr);
    f_muntrace = M.getOrInsertFunction(
        "muntrace", attrs, Type::getVoidTy(M.getContext()), nullptr);
    Function *tmp = cast<Function>(f_mtrace);
    tmp->setCallingConv(CallingConv::C);
    tmp = cast<Function>(f_muntrace);
    tmp->setCallingConv(CallingConv::C);

    for (Function &F : M) {
      if (F.getName() != "main") continue;
      CallInst::Create(f_mtrace, "", F.front().getFirstNonPHIOrDbg());
      CallInst::Create(f_muntrace, "", F.back().getTerminator());
    }
    return true;
  }
};
}  // namespace

char AddMtrace::ID = 0;
static RegisterPass<AddMtrace> X("AddMtrace", "AddMtrace");
