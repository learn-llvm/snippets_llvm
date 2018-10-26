#ifndef IR_UTILS_H
#define IR_UTILS_H

namespace llvm {
    class CallInst;

    class Constant;

    class Function;

    class FunctionType;

    class GlobalVariable;

    class Instruction;

    class Module;

    class DataLayout;

    class Type;

    class Value;

    namespace utils {
        bool isTrivialPointer(Value const *const V);

        bool isConstantValue(Value const *const V);

        bool isPointerValue(Value const *const V);

        bool isPointerToPointerValue(Value const *const V);

        bool isPointerManipulation(Instruction const *const I);

        Type const *getPointedType(Value const *const V);

        Type const *getPointedType(Type const *T);

        bool isGlobalPointerInit(GlobalVariable const *const G);

        FunctionType const *getCalleeFnType(CallInst const *const C);

        bool isFn_malloc(Function const *const F);

        bool isFn_free(Function const *const F);

        bool isFn_memcpy(Function const *const F);

        bool isFn_memmove(Function const *const F);

        bool isFn_memset(Function const *const F);

        bool isFn_mem_ops(Function const *const F);

        bool is_inlineAsm_withSideEffect(CallInst const *CI);

        bool isInst_mem_ops(CallInst const *const C);

        Instruction const *getFunctionEntry(Function const *F);

        bool isLocalToFunction(Value const *const V, Function const *const F);

        bool callToVoidFunction(CallInst const *const C);

        Instruction const *getSuccInBlock(Instruction const *const);

        Instruction const *getPredInBlock(Instruction const *const);

        Value *elimConstExpr(Value *V);

        Value const *elimConstExpr(const Value *V);

        unsigned getTypeSize(DataLayout const &targetData, Type const *type);

        bool isIntegerRelatedType(Type const *ty);
    }  //  namespace llvm::utils

}  // namespace llvm

#endif
