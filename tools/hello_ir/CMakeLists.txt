project(hello_ir.out)

link_directories(${LLVM_LIBRARY_DIR})

add_executable(${PROJECT_NAME} hello_ir.cc)

# set(FULL_LLVM_LIBS
#   -lLLVMX86CodeGen -lLLVMX86Desc -lLLVMX86Info -lLLVMX86AsmPrinter -lLLVMX86Utils -lLLVMSelectionDAG -lLLVMAsmPrinter -lLLVMMCParser -lLLVMInterpreter -lLLVMJIT -lLLVMRuntimeDyld -lLLVMExecutionEngine -lLLVMCodeGen -lLLVMObjCARCOpts -lLLVMScalarOpts -lLLVMInstCombine -lLLVMTransformUtils -lLLVMipa -lLLVMAnalysis -lLLVMTarget -lLLVMMC -lLLVMObject -lLLVMCore -lLLVMSupport -lz -lpthread -ltinfo -ldl -lm
#   )

target_link_libraries(${PROJECT_NAME} ${FULL_LLVM_LIBS} mybase)
