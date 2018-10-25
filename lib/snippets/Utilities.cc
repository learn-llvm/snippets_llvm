#include "Utilities.hh"
#include "Version.hh"

void dumpLinkageType(GlobalValue &GV) {
  GlobalValue::LinkageTypes lty = GV.getLinkage();
#define dumpLTYInfo(key, lty) \
  errs() << #key << " " << GlobalValue::is##key(lty) << "\n"

#if LLVM_VERSION_CODE >= LLVM_VERSION(3, 5)
#define LIST                                                                  \
  X(ExternalLinkage) X(AvailableExternallyLinkage) X(LinkOnceLinkage)         \
      X(WeakLinkage) X(AppendingLinkage) X(InternalLinkage) X(PrivateLinkage) \
          X(PrivateLinkage) X(LocalLinkage) X(ExternalLinkage)                \
              X(CommonLinkage) X(DiscardableIfUnused) X(WeakForLinker)
#else
#define LIST                                                                  \
  X(ExternalLinkage) X(AvailableExternallyLinkage) X(LinkOnceLinkage)         \
      X(WeakLinkage) X(AppendingLinkage) X(InternalLinkage) X(PrivateLinkage) \
          X(LinkerPrivateLinkage) X(LocalLinkage) X(DLLImportLinkage)         \
              X(ExternalLinkage) X(CommonLinkage) X(DiscardableIfUnused)      \
                  X(WeakForLinker)
#endif
#define X(name) dumpLTYInfo(name, lty);
  LIST;
#undef X
  errs() << "maybeOverridden " << GlobalValue::mayBeOverridden(lty) << "\n";
}

void dumpGVInfo(GlobalValue &GV) {
  logging::prettyPrint(&GV);
  errs() << GV.getRealLinkageName(GV.getName()) << "\n";
  errs() << "isMaterializable: " << GV.isMaterializable() << "\n";
  errs() << "isDematerializable: " << GV.isDematerializable() << "\n";
  dumpLinkageType(GV);
  errs() << "\n";
}

void dumpPassKind(PassKind kind) {
  std::string kindStr;
  switch (kind) {
    case PT_BasicBlock:
      kindStr = "bb";
      break;
    case PT_Region:
      kindStr = "region";
      break;
    case PT_Loop:
      kindStr = "loop";
      break;
    case PT_Function:
      kindStr = "func";
      break;
    case PT_CallGraphSCC:
      kindStr = "cgscc";
      break;
    case PT_Module:
      kindStr = "module";
      break;
    case PT_PassManager:
      kindStr = "pm";
      break;
  }
  errs() << "passKind: " << kindStr << "\n";
}
