project(hello_ir.out)

llvm_map_components_to_libnames(llvm_libs bitwriter core ipo irreader instcombine instrumentation target linker analysis scalaropts support
        x86asmparser x86asmprinter x86codegen x86desc x86disassembler x86info)

add_executable(${PROJECT_NAME} Gen.cc)

target_link_libraries(${PROJECT_NAME} mybase ${llvm_libs})
