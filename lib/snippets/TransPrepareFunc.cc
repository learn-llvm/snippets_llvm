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

#include "IRUtils.hh"
#include "Logging.hh"

namespace llvm {
    class PrepareFunc : public ModulePass {
        Function const *assertFail{};
        Function *main{};

        bool POSIXizeMain(Module &M) {
            main = M.getFunction("main");
            if (main == nullptr) {
                errs() << "no main function in " << M.getModuleIdentifier() << "\n";
                return false;
            }
            if (main->arg_size() == 2) {
                errs() << "arg_size == 2\n";
                FunctionType const *ft = main->getFunctionType();
                Type const *argc = ft->getParamType(0);
                assert(argc->getTypeID() == Type::IntegerTyID);
                Type const *argv = ft->getParamType(1);
                auto const *argv_e =
                        cast<PointerType const>(utils::getPointedType(argv));
                assert(argv_e->getElementType()->isIntegerTy());
                assert(cast<IntegerType>(argv_e->getElementType())->getBitWidth() == 8);
                return false;
            }
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

    public:
        static char ID;

        PrepareFunc() : ModulePass(ID) {}

        bool runOnModule(Module &M) override {
            bool changed = POSIXizeMain(M);
            assertFail = M.getFunction("__assert_fail");
            if (!assertFail) return false;
            assert(assertFail->hasFnAttribute(Attribute::NoReturn) &&
                   assertFail->hasFnAttribute(Attribute::NoUnwind));
            for (Function &F : M) {
                if (F.isDeclaration()) continue;
                changed |= runOnFunction(F);
            }
            return changed;
        }

        bool runOnFunction(Function &F) {
            bool isNeeded = false;
            for (auto &B : F) {
                for (auto &I : B) {
                    if (isa<CallInst>(&I) && !isa<IntrinsicInst>(&I)) {
                        auto &callInst = cast<CallInst>(I);
                        auto *calledFunc = callInst.getCalledFunction();
                        if (calledFunc == assertFail) {
                            auto &branchInst =
                                    cast<BranchInst>(callInst.getParent()->getPrevNode()->back());
                            assert(branchInst.isConditional());
                            isNeeded = true;
                        }
                    }
                }
            }
            return isNeeded;
        }

        void getAnalysisUsage(AnalysisUsage &AU) const override {
            AU.setPreservesAll();
        }
    };

    char PrepareFunc::ID = 0;
    static RegisterPass<PrepareFunc> X("PrepareFunc", "PrepareFunc", false, false);
}
