#define DEBUG_TYPE "PrepareFunc"

#include "llvm/Support/Debug.h"

#include "llvm/IR/Attributes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/TypeBuilder.h"

#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include "LLDump.hh"
#include "LLUtils.hh"

namespace llvm {
class PrepareFunc : public ModulePass {
  Function *main = nullptr;

  bool POSIXizeMain(Module &M) {
    main = M.getFunction("main");
    if (main == nullptr) {
      errs() << "no main function in " << M.getModuleIdentifier() << "\n";
      return false;
    }
    if (main->arg_size() == 2) {
      errs() << "arg_size == 2, nothing to do\n";
      FunctionType const *ft = main->getFunctionType();
      Type const *argc = ft->getParamType(0);
      assert(argc->getTypeID() == Type::IntegerTyID);
      Type const *argv = ft->getParamType(1);
      auto const *argv_e = cast<PointerType const>(utils::getPointedType(argv));
      assert(argv_e->getElementType()->isIntegerTy());
      assert(cast<IntegerType>(argv_e->getElementType())->getBitWidth() == 8);
      return false;
    } else {
      assert(main->arg_size() == 0);
      main->setName("old_main");
      FunctionType *mainTy =
          TypeBuilder<int(int, char **), false>::get(M.getContext());
      auto *newMain = Function::Create(mainTy, main->getLinkage(), "main", &M);
      newMain->setAttributes(main->getAttributes());
      newMain->getBasicBlockList().splice(newMain->begin(),
                                          main->getBasicBlockList());
      main->eraseFromParent();
      return true;
    }
  }

 public:
  static char ID;

  PrepareFunc() : ModulePass(ID) {}

  bool runOnModule(Module &M) override {
    dumpPassKind(this->getPassKind());
    errs() << this->getPassName() << " ID:" << this->getPassID() << "\n";
    return POSIXizeMain(M);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }
};

char PrepareFunc::ID = 0;
static RegisterPass<PrepareFunc> X("posix-main", "make it a posix main", false, false);
}  // namespace llvm
