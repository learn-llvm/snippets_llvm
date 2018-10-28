project(ir_dump.out)
llvm_map_components_to_libnames(llvm_libs bitwriter core ipo irreader instcombine instrumentation target linker analysis scalaropts support)
add_executable(${PROJECT_NAME} Dump.cc)
target_link_libraries(${PROJECT_NAME} ${llvm_libs} mybase)