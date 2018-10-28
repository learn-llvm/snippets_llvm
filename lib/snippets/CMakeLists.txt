PROJECT(snippets)

add_llvm_loadable_module(countbb CountExecBB.cc)
target_link_libraries(countbb mybase)
add_llvm_loadable_module(cfgpath DumpCFGPath.cc)
add_llvm_loadable_module(localopt LocalOpts.cc)
add_llvm_loadable_module(user DumpUseDef.cc)
add_llvm_loadable_module(prepare POSIXMain.cc)
