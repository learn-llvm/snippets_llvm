link_directories(${LLVM_LIBRARY_DIR})

set(SUB_PROJECTS
  DS
  # klee
  # hello_ir
  bb_topsort
  rtti
  # simple
  # toy
  # intrinsics
  yaml
  )


foreach(MODULE ${SUB_PROJECTS})
  add_subdirectory(${MODULE})
endforeach()
