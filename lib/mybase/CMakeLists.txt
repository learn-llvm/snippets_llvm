project(mybase)
add_llvm_library(${PROJECT_NAME}
        LLUtils.cc
        Common.cc
        LLDump.cc)
llvm_map_components_to_libnames(llvm_libs bitwriter core ipo irreader instcombine instrumentation target linker analysis scalaropts support
        x86asmparser x86asmprinter x86codegen x86desc x86disassembler x86info)
target_link_libraries(${PROJECT_NAME} ${llvm_libs})