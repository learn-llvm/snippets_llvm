function(replace_compiler_option var old new)
  # Replaces a compiler option or switch `old' in `var' by `new'.
  # If `old' is not in `var', appends `new' to `var'.
  # Example: replace_compiler_option(CMAKE_CXX_FLAGS_RELEASE "-O3" "-O2")
  # If the option already is on the variable, don't add it:
  if( "${${var}}" MATCHES "(^| )${new}($| )" )
    set(n "")
  else()
    set(n "${new}")
  endif()
  if( "${${var}}" MATCHES "(^| )${old}($| )" )
    string( REGEX REPLACE "(^| )${old}($| )" " ${n} " ${var} "${${var}}" )
  else()
    set( ${var} "${${var}} ${n}" )
  endif()
  set( ${var} "${${var}}" PARENT_SCOPE )
endfunction()

macro (setup_package_version_variables _packageName)
  if (DEFINED ${_packageName}_VERSION)
    string (REGEX MATCHALL "[0-9]+" _versionComponents "${${_packageName}_VERSION}")
    list (LENGTH _versionComponents _len)
    if (${_len} GREATER 0)
      list(GET _versionComponents 0 ${_packageName}_VERSION_MAJOR)
    endif()
    if (${_len} GREATER 1)
      list(GET _versionComponents 1 ${_packageName}_VERSION_MINOR)
    endif()
    if (${_len} GREATER 2)
      list(GET _versionComponents 2 ${_packageName}_VERSION_PATCH)
    endif()
    if (${_len} GREATER 3)
      list(GET _versionComponents 3 ${_packageName}_VERSION_TWEAK)
    endif()
    set (${_packageName}_VERSION_COUNT ${_len})
  else()
    set (${_packageName}_VERSION_COUNT 0)
    set (${_packageName}_VERSION "")
  endif()
endmacro()

find_program(LLVM_CONFIG_EXECUTABLE llvm-config${LLVM_SUFFIX})

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --version
  OUTPUT_VARIABLE LLVM_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )
string(REGEX MATCH "[0-9]+\\.[0-9]+" LLVM_VERSION ${LLVM_VERSION})

if (NOT LLVM_CONFIG_EXECUTABLE)
  message(FATAL_ERROR "Could not find llvm-config")
endif ()

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --src-root
  OUTPUT_VARIABLE LLVM_SRC_ROOT
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --obj-root
  OUTPUT_VARIABLE LLVM_OBJ_ROOT
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --includedir
  OUTPUT_VARIABLE LLVM_INCLUDE_DIR
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --libdir
  OUTPUT_VARIABLE LLVM_LIBRARY_DIR
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --cflags
  OUTPUT_VARIABLE LLVM_CFLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --cxxflags
  OUTPUT_VARIABLE LLVM_CXXFLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --ldflags
  OUTPUT_VARIABLE LLVM_LDFLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --libs
  OUTPUT_VARIABLE LLVM_MODULE_LIBS
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --system-libs
  OUTPUT_VARIABLE LLVM_SYSTEM_LIBS
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
  )
string(COMPARE EQUAL "${LLVM_SYSTEM_LIBS}" "" SYSTEM_LIBS_UNDEFINED)
if(SYSTEM_LIBS_UNDEFINED)
  set(LLVM_SYSTEM_LIBS "-lz -lpthread -ltinfo -ldl -lm")
endif()

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --bindir
  OUTPUT_VARIABLE LLVM_BINARY_DIR
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

replace_compiler_option(LLVM_CFLAGS "-DNDEBUG" "-UNDEBUG")
foreach(OPTLEVEL "-O" "-O0" "-O1" "O2" "-O3" "-O4" "-Os")
  replace_compiler_option(LLVM_CFLAGS ${OPTLEVEL} "")
endforeach()

if(NOT WIN32)
  string(ASCII 27 Esc)
  set(ColourReset "${Esc}[m")
  set(ColourBold  "${Esc}[1m")
  set(Red         "${Esc}[31m")
  set(Green       "${Esc}[32m")
  set(Yellow      "${Esc}[33m")
  set(Blue        "${Esc}[34m")
  set(Magenta     "${Esc}[35m")
  set(Cyan        "${Esc}[36m")
  set(White       "${Esc}[37m")
  set(BoldRed     "${Esc}[1;31m")
  set(BoldGreen   "${Esc}[1;32m")
  set(BoldYellow  "${Esc}[1;33m")
  set(BoldBlue    "${Esc}[1;34m")
  set(BoldMagenta "${Esc}[1;35m")
  set(BoldCyan    "${Esc}[1;36m")
  set(BoldWhite   "${Esc}[1;37m")
endif()

message(STATUS "----------------------------------------------------------")
message(STATUS "${Magenta}LLVM version${ColourReset}: ${LLVM_VERSION}")
message(STATUS "${Magenta}LLVM src-root${ColourReset}: ${LLVM_SRC_ROOT}")
message(STATUS "${Magenta}LLVM obj-root${ColourReset}: ${LLVM_OBJ_ROOT}")
message(STATUS "${Magenta}LLVM includedir${ColourReset}: ${LLVM_INCLUDE_DIR}")
message(STATUS "${Magenta}LLVM bindir${ColourReset}: ${LLVM_BINARY_DIR}")
message(STATUS "${Magenta}LLVM libdir${ColourReset}: ${LLVM_LIBRARY_DIR}")
message(STATUS "${Magenta}LLVM cflags${ColourReset}: ${LLVM_CFLAGS}")
message(STATUS "${Magenta}LLVM cxxflags${ColourReset}: ${LLVM_CXXFLAGS}")
message(STATUS "${Magenta}LLVM ldflags${ColourReset}: ${LLVM_LDFLAGS}")
message(STATUS "${Magenta}LLVM libs${ColourReset}: ${LLVM_MODULE_LIBS}")
message(STATUS "${Magenta}LLVM system-libs${ColourReset}: ${LLVM_SYSTEM_LIBS}")
message(STATUS "----------------------------------------------------------")
