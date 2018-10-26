llvm_map_components_to_libnames(llvm_libs bitwriter core ipo irreader instcombine instrumentation target linker analysis scalaropts support)
add_executable(simple simple.cc)
target_link_libraries(simple ${llvm_libs} mybase)