PROJECT(snippets)

add_llvm_loadable_module(countbb CountExecBB.cc)
add_llvm_loadable_module(cfgpath DumpCFGPath.cc)
add_llvm_loadable_module(localopt LocalOpts.cc)
add_llvm_loadable_module(of TestOF.cc)
add_llvm_loadable_module(user DumpUseDef.cc)
add_llvm_loadable_module(addtrace AddMTrace.cc)
add_llvm_loadable_module(prepare TransPrepareFunc.cc)
