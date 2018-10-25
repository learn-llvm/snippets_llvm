project(klee.out)

file(GLOB SRC_FILES *.cc)

include_directories("/home/hongxu/RESEARCH/klee/src/include")
include_directories("/home/hongxu/RESEARCH/klee/obj/include")

# message(STATUS "${CMAKE_SHARED_LIBRARY_LINK_C_FLAGS}")
# SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
# message(STATUS "${CMAKE_SHARED_LIBRARY_LINK_C_FLAGS}")

link_directories("/home/hongxu/RESEARCH/klee/obj/Release+Asserts/lib/" "/home/hongxu/RESEARCH/stp/install/lib/")

set(KLEE_LIBS
  kleeCore
  kleeBasic
  kleeModule
  kleaverSolver
  kleaverExpr
  kleeSupport
  )

set(STP_LIBS
  stp
  )

# set(CMAKE_VERBOSE_MAKEFILE 1)

add_executable(${PROJECT_NAME} ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} mybase ${KLEE_LIBS} ${STP_LIBS} ${FULL_LLVM_LIBS})
