//------------------------------------------------------------------------------
/// define i32 @func(i32 %a, i32 %b) #0 {
/// AA:
///   %cmp1 = icmp eq i32 %a, %b
///   br i1 %cmp1, label %BB, label %CC
/// BB:
///   %cmp2 = icmp ult i32 %a, %b
///   br i1 %cmp2, label %CC, label %DD
/// CC:
///   br label %DD
/// DD:
///   ret i32 %a
/// }

// OUTPUT should be:
/// Topological sort of func:
///   AA
///   BB
///   CC
///   DD

/// Basic blocks of func in post-order:
///   DD
///   CC
///   BB
///   AA

//------------------------------------------------------------------------------
/// define i32 @func(i32 %a, i32 %b) #0 {
/// AA:
///   %cmp1 = icmp eq i32 %a, %b
///   br i1 %cmp1, label %BB, label %CC
/// BB:
///   br label %BB1
/// BB1:
///   br label %BB2
/// BB2:
///   %cmp4 = icmp eq i32 %a, %b
///   br i1 %cmp4, label %BB3, label %BB
/// BB3:
///   br label %BB4
/// BB4:
///   %cmp3 = icmp eq i32 %a, %b
///   br i1 %cmp3, label %BB3, label %CC
/// CC:
///   br label %CC1
/// CC1:
///   br label %CC2
/// CC2:
///   %cmp2 = icmp eq i32 %a, %b
///   br i1 %cmp2, label %DD, label %CC
/// DD:
///   ret i32 %a
/// }

/// Topological sort of func:
///   Detected cycle: edge from BB4 to BB3
///   Sorting failed

/// SCCs for func in post-order:
///   SCC: DD
///   SCC: CC2  CC1  CC
///   SCC: BB4  BB3
///   SCC: BB2  BB1  BB
///   SCC: AA

//------------------------------------------------------------------------------
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/PassManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/SourceMgr.h"
#include <string>
#include <vector>

#include "Version.hh"

#if LLVM_VERSION_CODE >= LLVM_VERSION(3, 5)
#include "llvm/IR/CFG.h"
#else
#include "llvm/Support/CFG.h"
#endif

using namespace llvm;

// Runs a topological sort on the basic blocks of the given function. Uses
// the simple recursive DFS from "Introduction to algorithms", with 3-coloring
// of vertices. The coloring enables detecting cycles in the graph with a simple
// test.
class TopoSorter {
 public:
  void runToposort(const Function &F) {
    outs() << "Topological sort of " << F.getName() << ":\n";
    // Initialize the color map by marking all the vertices white.
    for (Function::const_iterator I = F.begin(), IE = F.end(); I != IE; ++I) {
      ColorMap[I] = TopoSorter::WHITE;
    }

    // The BB graph has a single entry vertex from which the other BBs should
    // be discoverable - the function entry block.
    bool success = recursiveDFSToposort(&F.getEntryBlock());
    if (success) {
      // Now we have all the BBs inside SortedBBs in reverse topological order.
      for (BBVector::const_reverse_iterator RI = SortedBBs.rbegin(),
                                            RE = SortedBBs.rend();
           RI != RE; ++RI) {
        outs() << "  " << (*RI)->getName() << "\n";
      }
    } else {
      outs() << "  Sorting failed\n";
    }
  }

 private:
  enum Color {
    WHITE,
    GREY,
    BLACK
  };
  // Color marks per vertex (BB).
  typedef DenseMap<const BasicBlock *, Color> BBColorMap;
  // Collects vertices (BBs) in "finish" order. The first finished vertex is
  // first, and so on.
  typedef SmallVector<const BasicBlock *, 32> BBVector;
  BBColorMap ColorMap;
  BBVector SortedBBs;

  // Helper function to recursively run topological sort from a given BB.
  // Returns true if the sort succeeded and false otherwise; topological sort
  // may fail if, for example, the graph is not a DAG (detected a cycle).
  bool recursiveDFSToposort(const BasicBlock *BB) {
    ColorMap[BB] = TopoSorter::GREY;
    // For demonstration, using the lowest-level APIs here. A BB's successors
    // are determined by looking at its terminator instruction.
    const TerminatorInst *TInst = BB->getTerminator();
    for (unsigned I = 0, NSucc = TInst->getNumSuccessors(); I < NSucc; ++I) {
      BasicBlock *Succ = TInst->getSuccessor(I);
      Color SuccColor = ColorMap[Succ];
      if (SuccColor == TopoSorter::WHITE) {
        if (!recursiveDFSToposort(Succ)) return false;
      } else if (SuccColor == TopoSorter::GREY) {
        // This detects a cycle because grey vertices are all ancestors of the
        // currently explored vertex (in other words, they're "on the stack").
        outs() << "  Detected cycle: edge from " << BB->getName() << " to "
               << Succ->getName() << "\n";
        return false;
      }
    }
    // This BB is finished (fully explored), so we can add it to the vector.
    ColorMap[BB] = TopoSorter::BLACK;
    SortedBBs.push_back(BB);
    return true;
  }
};

class AnalyzeBBGraph : public FunctionPass {
 public:
  AnalyzeBBGraph(const std::string &AnalysisKind)
      : FunctionPass(ID), AnalysisKind(AnalysisKind) {}

  virtual bool runOnFunction(Function &F) {
    if (AnalysisKind == "-topo") {
      TopoSorter TS;
      TS.runToposort(F);
    } else if (AnalysisKind == "-po") {
      // Use LLVM's post-order iterator to produce a reverse topological sort.
      // Note that this doesn't detect cycles so if the graph is not a DAG, the
      // result is not a true topological sort.
      outs() << "Basic blocks of " << F.getName() << " in post-order:\n";
      for (po_iterator<BasicBlock *> I = po_begin(&F.getEntryBlock()),
                                     IE = po_end(&F.getEntryBlock());
           I != IE; ++I) {
        outs() << "  " << (*I)->getName() << "\n";
      }
    } else if (AnalysisKind == "-scc") {
      // Use LLVM's Strongly Connected Components (SCCs) iterator to produce
      // a reverse topological sort of SCCs.
      outs() << "SCCs for " << F.getName() << " in post-order:\n";
      for (scc_iterator<Function *> I = scc_begin(&F), IE = scc_end(&F);
           I != IE; ++I) {
        // Obtain the vector of BBs in this SCC and print it out.
        const std::vector<BasicBlock *> &SCCBBs = *I;
        outs() << "  SCC: ";
        for (const auto &SCCBB : SCCBBs) {
          outs() << (SCCBB)->getName() << "  ";
        }
        outs() << "\n";
      }
    } else {
      outs() << "Unknown analysis kind: " << AnalysisKind << "\n";
    }
    return false;
  }

  // The address of this member is used to uniquely identify the class. This is
  // used by LLVM's own RTTI mechanism.
  static char ID;

 private:
  std::string AnalysisKind;
};

char AnalyzeBBGraph::ID = 0;

int main(int argc, char **argv) {
  if (argc < 3) {
    // Using very basic command-line argument parsing here...
    errs() << "Usage: " << argv[0] << " -[topo|po|scc] <IR file>\n";
    return 1;
  }

  // Parse the input LLVM IR file into a module.
  SMDiagnostic Err;
#if LLVM_VERSION_CODE >= LLVM_VERSION(3, 5)
  std::unique_ptr<Module> Mod(parseIRFile(argv[2], Err, getGlobalContext()));
#else
  std::unique_ptr<Module> Mod(ParseIRFile(argv[2], Err, getGlobalContext()));
#endif
  if (!Mod) {
    Err.print(argv[0], errs());
    return 1;
  }

  // Create a pass manager and fill it with the passes we want to run.
  PassManager PM;
  PM.add(new AnalyzeBBGraph(std::string(argv[1])));
  PM.run(*Mod);

  return 0;
}
