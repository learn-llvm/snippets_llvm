#define DEBUG_TYPE "hashdce"

#include "llvm/Pass.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"

#include "Version.hh"

#if LLVM_VERSION_CODE >= LLVM_VERSION(3, 5)
#include "llvm/IR/InstIterator.h"
#else
#include "llvm/Support/InstIterator.h"
#endif

#include "llvm/ADT/Statistic.h"
#include "llvm/Support/raw_ostream.h"
#include <map>
#include <queue>
#include <stack>
#define SAVE 0
#define RESTORE 1

/// https://github.com/rahul27/HashDCE

using namespace llvm;

STATISTIC(HashDCECounter, "Counts number of functions greeted");

class CP_Table_Stack_Entry {
  Value *key;
  Value *entry;
  int Depth;  // enhancement for multilevel nesting
 public:
  CP_Table_Stack_Entry(Value *key_val, Value *entry_val) {
    key = key_val;
    entry = entry_val;
  }
  Value *get_Key() { return key; }
  Value *get_Entry() { return entry; }
};

namespace {
struct HashDCE : public FunctionPass {

  static char ID;  // Pass identification, replacement for typeid
  HashDCE() : FunctionPass(ID) {}
  // most confusing part, naming conventions not right
  // why is i required
  // CP table includes CP<The actual location, Its alias that can be used>
  std::map<Value *, Value *> CP_Table;
  std::stack<CP_Table_Stack_Entry> CP_Table_Stack;
  std::map<Value *, bool> Mark;
  std::queue<Value *> WorkList;
  std::queue<Value *> IncList;
  std::map<Value *, int> HT;
  std::map<Value *, int> StatementContext;  // saved the value of a variable
  std::map<Value *, int> MarkContext;       // use to save the marked stores
  unsigned int IfElseRestore;

  virtual bool runOnFunction(Function &F) override {
    ++HashDCECounter;
    for (Function::iterator I = F.begin(), E = F.end(); I != E; I++) {
      BasicBlock *bb = I;
      Instruction &be = bb->back();
      bool CP_Table_status;
      std::queue<Value *> UseList;
      if (isa<BranchInst>(be)) {
        errs() << bb->back() << '\n';
        BranchInst *bi = dyn_cast<BranchInst>(&be);
        if (bi->isConditional()) {
          // errs() << "iscondi"<<'\n';
          CP_Table_status = SAVE;
          IfElseRestore = 1;
        } else
          CP_Table_status = RESTORE;
        // errs() <<"Jus another brch"<<"\n";
      }
      for (BasicBlock::iterator BI = bb->begin(), BE = bb->end(); BI != BE;
           BI++) {
        Instruction *i = BI;
        std::map<Value *, Value *>::iterator ii;
        if (isa<LoadInst>(i)) {
          ii = CP_Table.find(i->getOperand(0));

          if (ii != CP_Table.end()) {
            CP_Table[i] = ii->second;
            UseList.push(i);
          } else {
            CP_Table[i->getOperand(0)] = i;
            UseList.push(i->getOperand(0));
          }

          std::map<Value *, int>::iterator hi;
          hi = HT.find(i->getOperand(0));
          if (hi != HT.end()) {
            StatementContext[i] = HT[i->getOperand(0)];
            errs() << "Context Save\n" << HT[i->getOperand(0)] << "\n";
          }
        } else if (isa<StoreInst>(i)) {
          ii = CP_Table.find(i->getOperand(1));
          if (ii != CP_Table.end()) {
            CP_Table[i->getOperand(1)] = ii->second;
            UseList.push(i->getOperand(1));
          } else {
            CP_Table[i->getOperand(1)] = i->getOperand(0);
            UseList.push(i->getOperand(1));
          }
          std::map<Value *, int>::iterator hi;
          hi = HT.find(i->getOperand(1));
          if (hi != HT.end()) {
            HT[i->getOperand(1)]++;
            errs() << "HT plus plus\n" << HT[i->getOperand(1)] << "\n";
            StatementContext[i] = HT[i->getOperand(1)];
            IncList.push(i->getOperand(1));
          }
        } else if (isa<CallInst>(i)) {
          i->getNumOperands();
        } else if (isa<BranchInst>(i)) {
          BranchInst *bi = dyn_cast<BranchInst>(i);
          unsigned int count = 0;
          if (bi->isConditional()) {
            errs() << "Branch Inst " << i->getNumOperands() << "\n";
            count = 0;
            while (count < i->getNumOperands()) {
              errs() << i->getOperand(count) << "\n";
              count++;
            }
          }
        } else if (isa<AllocaInst>(i)) {
          errs() << "Alloca " << *(i) << "\n";
          HT[i] = 0;
        }
        errs() << "\n" << i << "\n" << *i << "\n";
      }
      errs() << "Block End && IfElse Restore " << IfElseRestore << "\n";
      if (CP_Table_status == RESTORE) {
        std::map<Value *, Value *>::iterator ii;
        while (!UseList.empty()) {
          Instruction *I = (Instruction *)UseList.front();
          UseList.pop();
          ii = CP_Table.find(I);
          if (ii != CP_Table.end()) {
            CP_Table.erase(I);
          }
          // Activate Later
        }
        if (IfElseRestore != 0) {
          while (!IncList.empty()) {
            std::map<Value *, int>::iterator hi;
            Instruction *I = (Instruction *)IncList.front();
            IncList.pop();
            hi = HT.find(I);
            if (hi != HT.end()) {

              HT[I]--;
            }
          }
        }
        IfElseRestore--;
        // Restore CP_TABLE and stack
        std::stack<CP_Table_Stack_Entry> CP_Table_Stack_Bkp;
        while (!CP_Table_Stack.empty()) {
          CP_Table_Stack_Entry s = CP_Table_Stack.top();
          CP_Table_Stack.pop();
          std::map<Value *, Value *>::iterator ii;
          ii = CP_Table.find(s.get_Key());
          if (ii == CP_Table.end()) {
            CP_Table[s.get_Key()] = s.get_Entry();
          }
          CP_Table_Stack_Bkp.push(s);
        }
        while (!CP_Table_Stack_Bkp.empty()) {
          CP_Table_Stack_Entry s = CP_Table_Stack_Bkp.top();
          CP_Table_Stack_Bkp.pop();
          CP_Table_Stack.push(s);
        }
      } else if (CP_Table_status == SAVE) {
        std::map<Value *, Value *>::iterator it;
        for (it = CP_Table.begin(); it != CP_Table.end(); it++) {
          CP_Table_Stack_Entry s(it->first, it->second);
          CP_Table_Stack.push(s);
        }
      }
    }
    errs().write_escaped(F.getName()) << '\n';
    for (inst_iterator I = inst_end(F), E = inst_begin(F); I != E; I--) {
      Instruction *i;
      i = &*I;
      if (i == nullptr) {
        continue;
      }
      if (isa<CallInst>(i) or Mark[i] or isa<ReturnInst>(
              i) or isa<TerminatorInst>(i)) {
        Mark[i] = true;
        unsigned int count = 0;
        if (isa<CallInst>(i)) {
          Mark[i->getOperand(1)] = true;
          errs() << "Marked" << *(i->getOperand(1)) << '\n';
        } else if (isa<LoadInst>(i)) {
          MarkContext[i->getOperand(0)] = StatementContext[i];
        } else {
          count = 0;
          while (count < i->getNumOperands()) {
            Mark[i->getOperand(count)] = true;
            errs() << "Marked" << *(i->getOperand(count)) << '\n';
            count++;
          }
        }
        errs() << "Marked" << *i << '\n';
      } else {
        unsigned int count = 0;
        if (isa<StoreInst>(i)) {
          std::map<Value *, int>::iterator hi;
          hi = MarkContext.find(i->getOperand(1));
          if (hi != HT.end()) {
            if (StatementContext[i] == MarkContext[i->getOperand(1)]) {
              Mark[i] = true;
              count = 0;
              while (count < i->getNumOperands()) {
                Mark[i->getOperand(count)] = true;
                errs() << "Marked " << *(i->getOperand(count)) << '\n';
                count++;
              }
            } else
              WorkList.push(i);
          } else
            WorkList.push(i);
        } else {
          WorkList.push(i);
        }
      }
    }
    while (!WorkList.empty()) {
      Instruction *I = (Instruction *)WorkList.front();
      WorkList.pop();
      I->eraseFromParent();
    }
    return false;
  }
};
}

char HashDCE::ID = 0;
static RegisterPass<HashDCE> X("hashdce", "HashDCE Pass");
