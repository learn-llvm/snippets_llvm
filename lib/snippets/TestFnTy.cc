#define DEBUG_TYPE "TestFnTy"

#include "llvm/Support/Debug.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"

#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include "Logging.hh"

using namespace llvm;

struct TestFnTy final : public ModulePass {
    static char ID;

    TestFnTy() : ModulePass(ID) {}

    void dumpFnTy(Function &F) {
        errs() << F.getName() << "\n";
        FunctionType *fnTy = F.getFunctionType();
        logging::printTypeInfo(fnTy);
        errs() << "no. of param types: " << fnTy->getNumParams()
               << "\nparameter typs:\n";
        for (auto *ty : fnTy->params()) {
            logging::printTypeInfo(ty);
        }
        errs() << "\n";
    }

    void getSymbolTable(Module &M) {
        ValueSymbolTable &symtbl = M.getValueSymbolTable();
    }

    bool runOnModule(Module &M) override {
        for (auto &F : M) {
            dumpFnTy(F);
        }
        return false;
    }

    void getAnalysisUsage(AnalysisUsage &AU) const override {
        AU.setPreservesAll();
    }
};

char TestFnTy::ID = 0;
static RegisterPass<TestFnTy> X("TestFnTy", "TestFnTy pass", true, true);
