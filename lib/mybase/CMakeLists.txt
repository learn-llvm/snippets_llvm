file(GLOB COMMON_FILES *.cc)

project(mybase)
add_library(${PROJECT_NAME} 
  ${COMMON_FILES}
  )
target_link_libraries(${PROJECT_NAME})
