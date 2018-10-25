#include "DF.hh"
#include "llvm/Pass.h"

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"

#include "llvm/ADT/BitVector.h"
#include "llvm/Support/raw_ostream.h"
/// #include "llvm/Assembly/Writer.h"

#include "Logging.hh"

#include <ostream>

using namespace llvm;

namespace {
struct EDTest final : public Dataflow<true>, public FunctionPass {
  static char ID;

  EDTest() : Dataflow<true>(), FunctionPass(ID) {
    index = new ValueMap<Value *, int>();
    r_index = new std::vector<Value *>();
    instOut = new ValueMap<Instruction *, BitVector *>();
  }
  // map from instructions/argument to their index in the bitvector
  ValueMap<Value *, int> *index;

  // map from index in bitvector back to instruction/argument
  std::vector<Value *> *r_index;

  // convenience
  int numTotal;
  int numArgs;

  // map from instructions to bitvector corresponding to program point AFTER
  // that instruction
  ValueMap<Instruction *, BitVector *> *instOut;

  virtual void meet(BitVector *op1, const BitVector *op2) override {
    // union
    *op1 |= *op2;
  }

  virtual void getBoundaryCondition(BitVector *entry) override {
    // in[b] = just the arguments if no predecessors / entry node
    *entry = BitVector(numTotal, false);
    for (int i = 0; i < numArgs; ++i) {
      (*entry)[i] = true;
    }
  }

  bool isDefinition(Instruction *ii) {
    // All other types of instructions are definitions
    /// InvokeInst
    if (isa<TerminatorInst>(ii)) return false;
    if (isa<StoreInst>(ii)) return false;
    if (CallInst *CI = dyn_cast<CallInst>(ii)) {
      if (CI->getCalledFunction()->getReturnType()->isVoidTy()) return false;
    }
    return true;
  }

  BitVector *initialInteriorPoint(BasicBlock const &bb) override {
    // out[b] = empty set initially
    return new BitVector(numTotal, false);
  }

  virtual bool runOnFunction(Function &F) override {
    numTotal = 0;
    numArgs = 0;

    // add function arguments to maps
    for (Argument &arg : F.getArgumentList()) {
      (*index)[&arg] = numArgs;
      r_index->push_back(&arg);
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

    // initialize instOut
    for (BasicBlock &B : F) {
      for (Instruction &I : B) {
        (*instOut)[&I] = new BitVector(numTotal, false);
      }
    }

    top = new BitVector(numTotal, false);

    /// base
    Dataflow<true>::runOnFunction(F);

    // print out instructions with reaching variables between each instruction
    displayResults(F);

    return false;
  }

  virtual BitVector *transfer(BasicBlock &bb) override {
    // we iterate over instructions beginning with in[bb]
    BitVector *prev = (*in)[&bb];

    // temporary variables for convenience
    BitVector *instVec = prev;  // for empty blocks
    Instruction *inst;

    for (Instruction &I : bb) {
      inst = &I;
      instVec = (*instOut)[inst];
      *instVec = *prev;
      // if this instruction is a new definition, add it
      if (isDefinition(inst)) (*instVec)[(*index)[inst]] = true;
      // if it is a phi node, kill the stuff
      if (isa<PHINode>(inst)) {
        PHINode *p = cast<PHINode>(inst);
        unsigned num = p->getNumIncomingValues();
        for (unsigned i = 0; i != num; ++i) {
          Value *v = p->getIncomingValue(i);
          if (isa<Instruction>(v) || isa<Argument>(v)) {
            (*instVec)[(*index)[v]] = false;
          }
        }
      }

      prev = instVec;
    }

    // return a copy of the final instruction's post-condition to put in out[bb]
    return new BitVector(*instVec);
  }

  virtual void displayResults(Function &F) {
    Function::iterator bi = F.begin(), be = (F.end());
    for (; bi != be; bi++) {

      WITH_COLOR(raw_ostream::GREEN, errs() << "\n" << bi->getName() << ":\n");

      // display in[bb]
      if (!isa<PHINode>(*(bi->begin()))) {
        printBV((*in)[&*bi]);
      }

      // iterate over remaining instructions except very last one
      // we don't print out[i] for the last one because we should actually print
      // out the result of the meet operator at those points, i.e. in[next
      // block]...
      BasicBlock::iterator ii = bi->begin(), ie = --(bi->end());
      for (; ii != ie; ii++) {
        logging::prettyPrint(&*ii);
        if (!isa<PHINode>(*(++ii))) {
          --ii;
          printBV((*instOut)[&*ii]);
        } else
          --ii;
      }
      errs() << *ii << "\n";  /// terminator inst
    }
    // ...if there are more blocks
    printBV((*out)[&*(--be)]);
  }

  virtual void printBV(BitVector *bv) {
    errs() << "{ ";
    for (int i = 0; i < numTotal; i++) {
      if ((*bv)[i]) {
        errs() << (*r_index)[i] << " ";
      }
    }
    errs() << "}\n";
  }
};

char EDTest::ID = 0;
static RegisterPass<EDTest> x("RDTest", "RDTest", false, false);
}
