#define DEBUG_TYPE "MyCG"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"

#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/CallPrinter.h"
#include "llvm/Analysis/DOTGraphTraitsPass.h"

using namespace llvm;

namespace cg {

class MyCG final : public ModulePass {
  std::unique_ptr<CallGraph> G;

 public:
  static char ID;

  MyCG() : ModulePass(ID) {}
  virtual ~MyCG() {}

  const CallGraph &getCallGraph() const { return *G; }
  CallGraph &getCallGraph() { return *G; }

  typedef CallGraph::iterator iterator;
  typedef CallGraph::const_iterator const_iterator;

  Module &getModule() const { return G->getModule(); }

  inline iterator begin() { return G->begin(); }
  inline iterator end() { return G->end(); }
  inline const_iterator begin() const { return G->begin(); }
  inline const_iterator end() const { return G->end(); }

  inline const CallGraphNode *operator[](const Function *F) const {
    return (*G)[F];
  }

  inline CallGraphNode *operator[](const Function *F) { return (*G)[F]; }

  CallGraphNode *getExternalCallingNode() const {
    return G->getExternalCallingNode();
  }

  CallGraphNode *getCallsExternalNode() const {
    return G->getCallsExternalNode();
  }

  Function *removeFunctionFromModule(CallGraphNode *CGN) {
    return G->removeFunctionFromModule(CGN);
  }

  CallGraphNode *getOrInsertFunction(const Function *F) {
    return G->getOrInsertFunction(F);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }

  bool runOnModule(Module &M) override {
    G.reset(new CallGraph(M));
    CallGraphNode *externCaller = G->getExternalCallingNode();
    externCaller->removeAllCalledFunctions();
    return false;
  }

  void releaseMemory() override { G.reset(); }

  void print(raw_ostream &OS, const Module *) const override {
    if (!G) {
      OS << "No call graph has been built!\n";
      return;
    }
    G->print(OS);
  }

  void dump() const { print(dbgs(), nullptr); }
};

char MyCG::ID = 0;
static RegisterPass<MyCG> X("MyCG", "MyCG pass", true, true);

//-------------------------------------------------------------------

struct MyCGTraits {
  static CallGraph *getGraph(MyCG *P) { return &P->getCallGraph(); }
};

//-------------------------------------------------------------------

struct MyCGP
    : public DOTGraphTraitsModulePrinter<MyCG, true, CallGraph *, MyCGTraits> {
  static char ID;

  MyCGP()
      : DOTGraphTraitsModulePrinter<MyCG, true, CallGraph *, MyCGTraits>(
            "callgraph", ID) {}

  void print(raw_ostream &OS, const Module *) const override {
    OS << "MyCGP Pass\n";
  }
};

char MyCGP::ID = 0;
static RegisterPass<MyCGP> Y("MyCGP", "MyCGP pass", true, true);

//-------------------------------------------------------------------

struct MyCGV
    : public DOTGraphTraitsModuleViewer<MyCG, true, CallGraph *, MyCGTraits> {
  static char ID;

  MyCGV()
      : DOTGraphTraitsModuleViewer<MyCG, true, CallGraph *, MyCGTraits>(
            "callgraph", ID) {}

  void print(raw_ostream &OS, const Module *) const override {
    OS << "MyCGV Pass\n";
  }
};

char MyCGV::ID = 0;
static RegisterPass<MyCGV> Z("MyCGV", "MyCGV pass", true, true);
//-------------------------------------------------------------------
}
