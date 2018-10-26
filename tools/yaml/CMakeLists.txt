llvm_map_components_to_libnames(llvm_libs bitwriter core ipo irreader instcombine instrumentation target linker analysis scalaropts support)
add_executable(yaml yaml.cc)
target_link_libraries(yaml ${llvm_libs} mybase)