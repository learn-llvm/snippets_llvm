#include "DF.hh"
#include "llvm/Pass.h"

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"

#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/ADT/BitVector.h"

#include "Logging.hh"

#include <ostream>

using namespace llvm;

namespace {
struct Liveness : public Dataflow<false>, public FunctionPass {
  static char ID;

  Liveness() : Dataflow<false>(), FunctionPass(ID) {
    index = new ValueMap<Value *, int>();
    r_index = new std::vector<Value *>();
    instIn = new ValueMap<Instruction const *, BitVector *>();
  }
  // map from instructions/argument to their index in the bitvector
  ValueMap<Value *, int> *index;

  // map from index in bitvector back to instruction/argument
  std::vector<Value *> *r_index;

  // convenience
  int numTotal;
  int numArgs;

  // map from instructions to bitvector corresponding to program point BEFORE
  // that instruction
  ValueMap<Instruction const *, BitVector *> *instIn;

  virtual void meet(BitVector *op1, const BitVector *op2) override {
    // union
    *op1 |= *op2;
  }

  virtual void getBoundaryCondition(BitVector *entry) override {
    // out[b] = empty set if no successors
    *entry = BitVector(numTotal, false);
  }

  bool isDefinition(Instruction *ii) {
    return (!(isa<TerminatorInst>(ii) || isa<StoreInst>(ii) ||
              (isa<CallInst>(ii) && cast<CallInst>(ii)
                                        ->getCalledFunction()
                                        ->getReturnType()
                                        ->isVoidTy())));
  }

  BitVector *initialInteriorPoint(BasicBlock const &bb) override {
    // in[b] = empty set initially
    return new BitVector(numTotal, false);
  }

  virtual bool runOnFunction(Function &F) override {
    numTotal = 0;
    numArgs = 0;

    // add function arguments to maps
    for (Function::arg_iterator ai = F.arg_begin(), ae = F.arg_end(); ai != ae;
         ai++) {
      (*index)[&*ai] = numArgs;
      r_index->push_back(&*ai);
      numArgs++;
    }
    numTotal = numArgs;

    // add definitions to maps
    for (BasicBlock &B : F) {
      for (Instruction &I : B) {
        if (!isDefinition(&I)) continue;
        (*index)[&I] = numTotal;
        r_index->push_back(&I);
        numTotal++;
      }
    }

    // initialize instIn
    for (BasicBlock const &BB : F) {
      for (Instruction const &I : BB) {
        (*instIn)[&I] = new BitVector(numTotal, false);
      }
    }
    top = new BitVector(numTotal, false);

    // run data flow
    Dataflow<false>::runOnFunction(F);

    // print out instructions with reaching variables between each instruction
    displayResults(F);

    // didn't modify nothing
    return false;
  }

  virtual BitVector *transfer(BasicBlock &bb) override {
    // we iterate over instructions in reverse beginning with out[bb]
    auto next = new BitVector(*((*out)[&bb]));

    // temporary variables for convenience
    BitVector *instVec = next;  // for empty blocks
    Instruction *inst;

    // add local phi-captured nodes to out
    for (Instruction const &I : bb) {
      if (!isa<PHINode>(&I)) continue;
      PHINode const *phiInst = cast<PHINode>(&I);
      unsigned idx = phiInst->getBasicBlockIndex(&bb);
      if (idx < phiInst->getNumIncomingValues()) {
        Value *v = phiInst->getIncomingValue(idx);
        if (isa<Instruction>(v) || isa<Argument>(v)) {
          (*next)[(*index)[v]] = true;
        }
      }
    }

    // go through instructions in reverse
    /// BUG, ud
    BasicBlock::iterator ii = --(bb.end()), ib = bb.begin();
    while (true) {

      // inherit data from next instruction
      inst = &*ii;
      instVec = (*instIn)[inst];
      *instVec = *next;

      // if this instruction is a new definition, remove it
      if (isDefinition(inst)) (*instVec)[(*index)[inst]] = false;

      // add the arguments, unless it is a phi node
      if (!isa<PHINode>(*ii)) {
        User::op_iterator OI, OE;
        for (OI = inst->op_begin(), OE = inst->op_end(); OI != OE; ++OI) {
          if (isa<Instruction>(*OI) || isa<Argument>(*OI)) {
            (*instVec)[(*index)[*OI]] = true;
          }
        }
      }
      next = instVec;

      if (ii == ib) break;
      --ii;
    }

    // remove the phi nodes from in
    instVec = new BitVector(*instVec);
    for (Instruction const &I : bb) {
      if (!isa<PHINode>(&I)) continue;
      PHINode const *phiInst = cast<PHINode>(&I);
      unsigned idx = phiInst->getBasicBlockIndex(&bb);
      if (idx < phiInst->getNumIncomingValues()) {
        Value *v = phiInst->getIncomingValue(idx);
        if (isa<Instruction>(v) || isa<Argument>(v)) {
          (*next)[(*index)[v]] = false;
        }
      }
    }
    return instVec;
  }

  virtual void displayResults(Function &F) {
    // iterate over basic blocks
    Function::iterator bi = F.begin(), be = (F.end());
    printBV((*out)[&*bi]);  // entry node
    for (; bi != be;) {
      WITH_COLOR(raw_ostream::CYAN, errs() << bi->getName() << ":\n");

      // iterate over remaining instructions except very first one
      BasicBlock::iterator ii = bi->begin(), ie = (bi->end());
      WITH_COLOR(raw_ostream::RED, errs() << "\t" << *ii << "\n");
      for (++ii; ii != ie; ++ii) {
        if (!isa<PHINode>(*(ii))) {
          printBV((*instIn)[&*ii]);
        }
        WITH_COLOR(raw_ostream::RED, errs() << "\t" << *ii << "\n");
      }

      // display in[bb]
      ++bi;

      if (bi != be && !isa<PHINode>(*((bi)->begin()))) printBV((*out)[&*bi]);

      errs() << "\n";
    }
    printBV((*out)[&*(--bi)]);
  }

  virtual void printBV(BitVector *bv) {
    errs() << "{ ";
    for (int i = 0; i < numTotal; i++) {
      if ((*bv)[i]) {
        errs() << ((*r_index)[i]) << " ";
      }
    }
    errs() << "}\n";
  }
};

char Liveness::ID = 0;
static RegisterPass<Liveness> x("Liveness", "Liveness", false, false);
}
