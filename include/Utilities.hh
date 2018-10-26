#ifndef TESTUTILITIES_HPP
#define TESTUTILITIES_HPP

#include "llvm/IR/GlobalValue.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Pass.h"

#include "Logging.hh"

using namespace llvm;
void dumpLinkageType(GlobalValue &GV);

void dumpGVInfo(GlobalValue &GV);

void dumpPassKind(PassKind kind);

#endif
