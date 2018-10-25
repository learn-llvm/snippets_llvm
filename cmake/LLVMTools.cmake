set(TEMP_BC_DIR /tmp/LLVM_BC)

function(CheckEmitLlvmFlag)
  file(WRITE "${TEMP_BC_DIR}/emitllvm.c" "#include<stdio.h>\n  int main(int argc, char* argv[]) {\n  puts(\"llvm c\\n\");\n  return 0;\n}\n\n")
  file(WRITE "${TEMP_BC_DIR}/emitllvm.cpp" "#include<iostream>\n  int main(int argc, char* argv[]) {\n  std::cout<<\"llvm cxx\\n\";\n  return 0;\n}\n\n")

  message(STATUS "Checking for LLVM C Front")
  execute_process(COMMAND "${LLVM_BC_C_COMPILER}" "-emit-llvm" "-c" "emitllvm.c" "-o" "emitllvm.c.bc"
    WORKING_DIRECTORY ${TEMP_BC_DIR}
    OUTPUT_QUIET ERROR_QUIET)
  execute_process(COMMAND "${LLVM_BC_ANALYZER}" "emitllvm.c.bc"
    WORKING_DIRECTORY ${TEMP_BC_DIR}
    RESULT_VARIABLE AOUT_IS_NOT_BC
    OUTPUT_QUIET ERROR_QUIET)
  if(AOUT_IS_NOT_BC)
    message(FATAL_ERROR "${LLVM_BC_C_COMPILER} is not valid LLVM compiler")
  endif()
  message(STATUS "Checking for LLVM C Front -- works.")

  message(STATUS "Checking for LLVM CXX Front")
 execute_process(COMMAND "${LLVM_BC_CXX_COMPILER}" "-emit-llvm" "-c" "emitllvm.cpp" "-o" "emitllvm.cpp.bc"
   WORKING_DIRECTORY ${TEMP_BC_DIR}
   OUTPUT_QUIET ERROR_QUIET)
 execute_process(COMMAND "${LLVM_BC_ANALYZER}" "emitllvm.cpp.bc"
   WORKING_DIRECTORY ${TEMP_BC_DIR}
   RESULT_VARIABLE AOUT_IS_NOT_BC
   OUTPUT_QUIET ERROR_QUIET)
 if(AOUT_IS_NOT_BC)
   message(FATAL_ERROR "${LLVM_BC_CXX_COMPILER} is not valid LLVM compiler")
 endif()
 message(STATUS "Checking for LLVM CXX Front -- works")
endfunction(CheckEmitLlvmFlag)

## ADD_BITCODE ##
macro(add_bitcode target)

  set(bcfiles "")
  foreach(srcfile ${ARGN})
    ## get the definitions, flags, and includes to use when compiling this file
    set(srcdefs "")
    get_directory_property(COMPILE_DEFINITIONS COMPILE_DEFINITIONS)
    foreach(DEFINITION ${COMPILE_DEFINITIONS})
      list(APPEND srcdefs -D${DEFINITION})
    endforeach()

    set(srcflags "")
    if(${srcfile} MATCHES "(.*).cpp")
      separate_arguments(srcflags UNIX_COMMAND ${CMAKE_CXX_FLAGS})
      set(src_bc_compiler ${LLVM_BC_CXX_COMPILER})
    else()
      separate_arguments(srcflags UNIX_COMMAND ${CMAKE_C_FLAGS})
      set(src_bc_compiler ${LLVM_BC_C_COMPILER} )
    endif()
    # if(NOT ${CMAKE_C_FLAGS} STREQUAL "")
    # string(REPLACE " " ";" srcflags ${CMAKE_C_FLAGS})
    # endif()

    set(srcincludes "")
    get_directory_property(INCLUDE_DIRECTORIES INCLUDE_DIRECTORIES)
    foreach(DIRECTORY ${INCLUDE_DIRECTORIES})
      list(APPEND srcincludes -I${DIRECTORY})
    endforeach()

    get_filename_component(outfile ${srcfile} NAME)
    get_filename_component(infile ${srcfile} ABSOLUTE)

    ## the command to generate the bitcode for this file
    add_custom_command(OUTPUT ${outfile}.bc
      COMMAND ${src_bc_compiler} -emit-llvm ${srcdefs} ${srcflags} ${srcincludes}
      -c ${infile} -o ${outfile}.bc
      COMMENT "Building LLVM bitcode ${outfile}.bc"
      VERBATIM
      )
    set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${outfile}.bc)

    ## keep track of every bitcode file we need to create
    list(APPEND bcfiles ${outfile}.bc)
  endforeach(srcfile)

  ## link all the bitcode files together to the target
  add_custom_command(OUTPUT ${target}.bc
    COMMAND ${LLVM_BC_LINK} ${BC_LD_FLAGS} -o ${CMAKE_CURRENT_BINARY_DIR}/${target}.bc ${bcfiles}
    DEPENDS ${bcfiles}
    COMMENT "Linking LLVM bitcode ${target}.bc"
    )
  set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${target}.bc)

  ## build all the bitcode files
  add_custom_target(${target} ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${target}.bc)
  set_property(TARGET ${target} PROPERTY LOCATION ${CMAKE_CURRENT_BINARY_DIR}/${target}.bc)

endmacro(add_bitcode)

## ADD_PLUGIN ##
macro(add_plugin target)

  set(bctargets "")
  foreach(bctarget ${ARGN})
    set(bcpath "")
    get_property(bcpath TARGET ${bctarget} PROPERTY LOCATION)
    if(${bcpath} STREQUAL "")
      message(FATAL_ERROR "Can't find property path for target '${bctarget}'")
    endif()
    list(APPEND bctargets ${bcpath})
  endforeach(bctarget)

  ## link all the bitcode targets together to the target, then hoist the globals
  add_custom_command(OUTPUT ${target}.bc
    COMMAND ${LLVM_BC_LINK} ${BC_LD_FLAGS} -o ${target}.bc ${bctargets}
    DEPENDS ${bctargets}
    COMMENT "Linking LLVM bitcode ${target}.bc"
    )
  set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${target}.bc)

  add_custom_command(OUTPUT ${target}.hoisted.bc
    COMMAND ${LLVM_BC_OPT} -load=${LLVMHoistGlobalsPATH} -hoist-globals ${target}.bc -o ${target}.hoisted.bc
    DEPENDS ${target}.bc LLVMHoistGlobals
    COMMENT "Hoisting globals from ${target}.bc to ${target}.hoisted.bc"
    )
  set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${target}.hoisted.bc)

  ## now we need the actual .so to be built
  add_library(${target} MODULE ${target}.hoisted.bc)
  add_dependencies(${target} ${target}.hoisted.bc)

  ## trick cmake so it builds the bitcode into a shared library
  set_property(TARGET ${target} PROPERTY LINKER_LANGUAGE C)
  set_property(SOURCE ${target}.hoisted.bc PROPERTY EXTERNAL_OBJECT TRUE)

  ## make sure we have the bitcode we need before building the .so
  foreach(bctarget ${ARGN})
    add_dependencies(${target} ${bctarget})
  endforeach(bctarget)

endmacro(add_plugin)


# -------------------------------------------------------------------------
find_program(LLVM_BC_C_COMPILER clang)
find_program(LLVM_BC_CXX_COMPILER clang++)
find_program(LLVM_BC_AR llvm-ar)
find_program(LLVM_BC_RANLIB llvm-ranlib)
find_program(LLVM_BC_LINK llvm-link)
find_program(LLVM_BC_ANALYZER llvm-bcanalyzer)
find_program(LLVM_BC_OPT opt)

foreach(LLVM_BC_EXE
    ${LLVM_BC_C_COMPILER}
    ${LLVM_BC_CXX_COMPILER}
    ${LLVM_BC_AR}
    ${LLVM_BC_RANLIB}
    ${LLVM_BC_LINK}
    ${LLVM_BC_ANALYZER}
    ${LLVM_BC_OPT})
  message(STATUS "Found LLVM ${LLVM_BC_EXE}")
endforeach()

CheckEmitLlvmFlag()
