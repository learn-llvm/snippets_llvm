#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/DataLayout.h"

#include "llvm/IR/CFG.h"

using namespace llvm;

template<typename T>
static std::string ToString(const T *obj) {
    std::string TypeName;
    raw_string_ostream N(TypeName);
    obj->print(N);
    return N.str();
}

struct SimpleModulePass : public ModulePass {
    static char ID;

    SimpleModulePass() : ModulePass(ID) {}

    void getAnalysisUsage(AnalysisUsage &AU) const override {
//        AU.addRequired<DataLayout>();
    }


    bool runOnBasicBlock(BasicBlock &BB, DataLayout const &TD) {
        for (auto &I : BB) {
            if (auto *Alloca = dyn_cast<AllocaInst>(&I)) {
                auto *AllocType = Alloca->getAllocatedType();
                AllocType->print(outs());
                outs() << " size " << TD.getTypeSizeInBits(AllocType) << " bits\n";
            }
            if (auto *GEP = dyn_cast<GetElementPtrInst>(&I)) {
                outs() << "Found GEP:\n";
                GEP->dump();
                outs() << "  The type is: " << ToString(GEP->getType()) << "\n";
                outs() << "  The pointer operand is: "
                       << ToString(GEP->getPointerOperand()) << "\n";
                outs() << "  Indices: ";
                for (auto Idx = GEP->idx_begin(), IdxE = GEP->idx_end(); Idx != IdxE;
                     ++Idx) {
                    outs() << "[" << ToString(Idx->get()) << "] ";
                }
                outs() << "\n";
            }
        }
        return false;
    }

    void printIntrinsic(Module &M) {
        for (auto &F : M) {
            for (auto &B : F) {
                for (auto &I : B) {
                    if (auto *CI = dyn_cast<CallInst>(&I)) {
                        Function *callee = CI->getCalledFunction();
                        if (callee != nullptr && callee->isIntrinsic()) {
                            errs() << I << "\n";
                        }
                    }
                }
            }
        }
    }

    void printInlineASM(Module &M) {
        for (auto &F : M) {
            for (auto &B : F) {
                for (auto &I : B) {
                    if (auto *CI = dyn_cast<CallInst>(&I)) {
                        if (auto *ia = dyn_cast<InlineAsm>(CI->getCalledValue())) {
                            errs() << "\n\ninline asm: " << *ia << "\n";
                            errs() << "asm string: ";
                            errs().write_escaped(ia->getAsmString());
                            errs() << "\nconstraint string: " << ia->getConstraintString()
                                   << "\n";
                            errs() << "hasSideEffects: " << ia->hasSideEffects() << "\n";
                            errs() << "isAlignStack: " << ia->isAlignStack() << "\n";
                            errs() << "dialect: " << (ia->getDialect() == InlineAsm::AD_ATT
                                                      ? "att"
                                                      : "intel") << "\n";
                            errs() << "Type: ";
                            ia->getType()->dump();
                            errs() << "\n";
                            errs() << "split asm string...\n";
                            SmallVector<StringRef, 4> asmPieces;
                        }
                    }
                }
            }
        }
    }

    void printPHIInfo(Module &M) {
        for (auto &F : M) {
            if (F.isDeclaration()) continue;
            for (auto &B : F) {
                for (BasicBlock::iterator I = B.begin(); isa<PHINode>(I); ++I) {
                    auto *phi = cast<PHINode>(I);
                    errs() << "PHI: " << *phi << "\n";
                    errs() << "incoming: " << phi->getNumIncomingValues() << "\n";
                    for (auto pred = pred_begin(&B); pred != pred_end(&B); ++pred) {
                        BasicBlock *B = *pred;
                        int index = phi->getBasicBlockIndex(B);
                        errs() << "\tindex of " << B->getName() << " is " << index << "\n";
                    }
                }
            }
        }
    }

    void printGVNames(Module &M) {
        for (auto GI = M.global_begin(), GE = M.global_end(); GI != GE; ++GI) {
            outs() << "Found global named \"" << GI->getName()
                   << "\": type = " << ToString(GI->getType()) << "\n";
        }
    }

    void printNMetadata(Module &M) {
        for (auto I = M.named_metadata_begin(), E = M.named_metadata_end(); I != E;
             ++I) {
            outs() << "Found MDNode:\n";
            I->dump();
            for (unsigned i = 0, e = I->getNumOperands(); i != e; ++i) {
                auto *Op = I->getOperand(i);
                if (auto *N = dyn_cast<MDNode>(Op)) {
                    outs() << "  Has MDNode operand:\n  ";
                    N->dump();
                    for (auto UI = N->op_begin(), UE = N->op_end(); UI != UE; ++UI) {
                        outs() << "   the operand has a user:\n    ";
                        outs() << *UI << "\n";
                    }
                }
            }
        }
    }

    bool runOnModule(Module &M) override {
        /// for (auto &&F : M)
        ///   for (auto &&B : F) runOnBasicBlock(B);
        /// printGVNames(M);
        /// printNMetadata(M);
        /// printPHIInfo(M);
        /// printInlineASM(M);
        printIntrinsic(M);
        return false;
    }
};

char SimpleModulePass::ID = 0;

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
    /// PM.add(new DataLayout());
    PM.add(new SimpleModulePass());
    PM.run(*Mod);

    return 0;
}
