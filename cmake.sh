#!/bin/bash

# First run the command as provided
cmake "$@"

# Parse the arguments
args=()
found=0
for arg in "$@"; do
  if [[ ($arg == "CodeBlocks - Unix Makefiles") ]]; then
    args+=("CodeBlocks - Ninja")
    found=1
  elif [[ ($arg != "-DCMAKE_MAKE_PROGRAM=/usr/bin/make") ]]; then
    args+=("$arg")
    found=1
  fi
done

# If make option arguments were found
if [[ $found == 1 ]]; then
  # Backup the Makefile.cmake required by Clion
  cp CMakeFiles/Makefile.cmake CMakeFiles/Makefile.cmake.bcp

  # Back-up the CMakeCache.txt and re-run for Ninja
  mv CMakeCache.txt CMakeCache.txt.make

  cmake "${args[@]}"

  cp CMakeCache.txt CMakeCache.txt.ninja

  # Restore the make baced files required by Clion
  mv CMakeFiles/Makefile.cmake.bcp CMakeFiles/Makefile.cmake
fi
