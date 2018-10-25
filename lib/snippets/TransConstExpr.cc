#include "llvm/IR/Instructions.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/GlobalVariable.h"

#include "llvm/Pass.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include "Version.hh"

#if LLVM_VERSION_CODE >= LLVM_VERSION(3, 5)
#include "llvm/IR/InstIterator.h"
#else
#include "llvm/Support/InstIterator.h"
#endif

#include "Logging.hh"

using namespace llvm;

struct LowerConstantExpr : public FunctionPass {
  static char ID;
  virtual bool runOnFunction(Function &F) override;

  void process(Instruction *inst);
  void lower(ConstantExpr *ce, Instruction *inst);

  LowerConstantExpr() : FunctionPass(ID) {}
};

FunctionPass *createLowerConstantExprPass() { return new LowerConstantExpr(); }

char LowerConstantExpr::ID = 0;

RegisterPass<LowerConstantExpr> X(
    "lower-const-expr", "Lower ConstantExprs to Insts to be eliminated");

/// ------------------------------------------------------------------------ ///

bool LowerConstantExpr::runOnFunction(Function &F) {
  logging::prettyPrint(&F, 1);
  for (auto &B : F) {
    auto ii = B.begin();
    while (ii != B.end()) {
      Instruction *inst = ii++;
      process(inst);
    }
  }

  return true;
}

void LowerConstantExpr::process(Instruction *inst) {
  for (auto oi = 0u, oe = inst->getNumOperands(); oi != oe; oi++) {
    auto *opnd = inst->getOperand(oi);
    if (auto *ce = dyn_cast<ConstantExpr>(opnd)) {
      lower(ce, inst);
    }
  }
}

void LowerConstantExpr::lower(ConstantExpr *ce, Instruction *inst) {
  Instruction *cinst = nullptr;

  unsigned opcode = ce->getOpcode();

  if (opcode == Instruction::GetElementPtr) {
    std::vector<Value *> idx;
    Value *ptr = ce->getOperand(0);

    for (auto i = 1u; i < ce->getNumOperands(); ++i)
      idx.push_back((Value *)ce->getOperand(i));

    cinst = GetElementPtrInst::Create(ptr, idx, "", inst);
  } else if (ce->isCast()) {
    cinst = CastInst::Create(Instruction::CastOps(opcode), ce->getOperand(0),
                             ce->getType(), "", inst);
  } else if (Instruction::isBinaryOp(opcode)) {
    cinst =
        BinaryOperator::Create(Instruction::BinaryOps(opcode),
                               ce->getOperand(0), ce->getOperand(1), "", inst);
  } else {
    assert(0 && "unhandled ConstantExpr");
  }

  inst->replaceUsesOfWith(ce, cinst);
  process(cinst);
}
