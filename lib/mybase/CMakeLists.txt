project(mybase)
set(BASE_SOURCES
        LLUtils.cc
        Common.cc
        LLDump.cc)
add_llvm_library(${PROJECT_NAME}
        ${BASE_SOURCES}
        )
target_link_libraries(${PROJECT_NAME})
set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
