#include "Utilities.hh"

void dumpLinkageType(GlobalValue &GV) {
    GlobalValue::LinkageTypes lty = GV.getLinkage();
#define dumpLTYInfo(key, lty) \
  errs() << #key << " " << GlobalValue::is##key(lty) << "\n"

#define LIST                                                                  \
  X(ExternalLinkage) X(AvailableExternallyLinkage) X(LinkOnceLinkage)         \
      X(WeakLinkage) X(AppendingLinkage) X(InternalLinkage) X(PrivateLinkage) \
          X(PrivateLinkage) X(LocalLinkage) X(ExternalLinkage)                \
              X(CommonLinkage) X(DiscardableIfUnused) X(WeakForLinker)

#define X(name) dumpLTYInfo(name, lty);
    LIST;
#undef X
}

void dumpGVInfo(GlobalValue &GV) {
    logging::prettyPrint(&GV);
    errs() << "isMaterializable: " << GV.isMaterializable() << "\n";
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
