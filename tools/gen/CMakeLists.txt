project(hello_ir.out)

link_directories(${LLVM_LIBRARY_DIR})

add_executable(${PROJECT_NAME} Gen.cc)

target_link_libraries(${PROJECT_NAME} mybase)
