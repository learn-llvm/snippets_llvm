llvm_map_components_to_libnames(llvm_libs bitwriter core ipo irreader instcombine instrumentation target linker analysis scalaropts support)
add_executable(ds DS.cc)
target_link_libraries(ds ${llvm_libs} mybase)