find_package(FLEX)
find_package(BISON)

add_definitions("-Wno-deprecated-register")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_BINARY_DIR})

flex_target(lexer tokens.l ${PROJECT_BINARY_DIR}/tokens.cc)
bison_target(parser parser.y ${PROJECT_BINARY_DIR}/parser.cc)
add_flex_bison_dependency(lexer parser)

project(toy.out)

set(SRCS
  ${BISON_parser_OUTPUTS}
  ${FLEX_lexer_OUTPUTS}
  codegen.cc
  corefn.cc
  main.cc
  )

add_executable(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} ${FULL_LLVM_LIBS})