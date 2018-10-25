#include <iostream>
#include "codegen.h"
#include "node.h"

extern int yyparse();
extern NBlock* programBlock;

Function* createPrintfFunction(CodeGenContext& context) {
  std::vector<Type*> printf_arg_types;
  printf_arg_types.push_back(Type::getInt8PtrTy(getGlobalContext()));  // char*

  FunctionType* printf_type = FunctionType::get(
      Type::getInt32Ty(getGlobalContext()), printf_arg_types, true);

  Function* func = Function::Create(printf_type, Function::ExternalLinkage,
                                    Twine("printf"), context.module.get());
  func->setCallingConv(CallingConv::C);
  return func;
}

void createEchoFunction(CodeGenContext& context, Function* printfFn) {
  std::vector<Type*> echo_arg_types;
  echo_arg_types.push_back(Type::getInt64Ty(getGlobalContext()));

  FunctionType* echo_type = FunctionType::get(
      Type::getVoidTy(getGlobalContext()), echo_arg_types, false);

  Function* func = Function::Create(echo_type, Function::InternalLinkage,
                                    Twine("echo"), context.module.get());
  BasicBlock* bblock = BasicBlock::Create(getGlobalContext(), "entry", func, 0);
  context.pushBlock(bblock);

  const char* constValue = "%d\n";
  Constant* format_const =
      ConstantDataArray::getString(getGlobalContext(), constValue);
  GlobalVariable* var = new GlobalVariable(
      *context.module, ArrayType::get(IntegerType::get(getGlobalContext(), 8),
                                      strlen(constValue) + 1),
      true, GlobalValue::PrivateLinkage, format_const, ".str");
  Constant* zero =
      Constant::getNullValue(IntegerType::getInt32Ty(getGlobalContext()));

  std::vector<Constant*> indices;
  indices.push_back(zero);
  indices.push_back(zero);
  Constant* var_ref = ConstantExpr::getGetElementPtr(var, indices);

  std::vector<Value*> args;
  args.push_back(var_ref);

  Function::arg_iterator argsValues = func->arg_begin();
  Value* toPrint = argsValues++;
  toPrint->setName("toPrint");
  args.push_back(toPrint);

  CallInst::Create(printfFn, makeArrayRef(args), "", bblock);
  ReturnInst::Create(getGlobalContext(), bblock);
  context.popBlock();
}

void createCoreFunctions(CodeGenContext& context) {
  Function* printfFn = createPrintfFunction(context);
  createEchoFunction(context, printfFn);
}
