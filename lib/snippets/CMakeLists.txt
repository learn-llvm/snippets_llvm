file(GLOB TEST_FILES *.cc dataflow/*.cc)

PROJECT(snippets)
add_library(${PROJECT_NAME} SHARED
  ${TEST_FILES}
  )
target_link_libraries(${PROJECT_NAME} mybase)