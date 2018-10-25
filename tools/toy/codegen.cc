/// echo 'int do_math(int a) { int x = a * 5 + 3 } do_math(10)'

#include "Version.hh"

#include "node.h"
#include "codegen.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/Interpreter.h"
#include "llvm/ExecutionEngine/JITEventListener.h"
#include "llvm/ExecutionEngine/JITMemoryManager.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "parser.hh"

/* Compile the AST into a module */
void CodeGenContext::generateCode(NBlock& root) {
  std::cout << "Generating code...\n";

  /* Create the top level interpreter function to call as entry */
  std::vector<Type*> argTypes;
  FunctionType* ftype = FunctionType::get(Type::getVoidTy(getGlobalContext()),
                                          makeArrayRef(argTypes), false);
  mainFunction = Function::Create(ftype, GlobalValue::InternalLinkage, "main",
                                  module.get());
  BasicBlock* bblock =
      BasicBlock::Create(getGlobalContext(), "entry", mainFunction, 0);

  /* Push a new variable/block context */
  pushBlock(bblock);
  root.codeGen(*this); /* emit bytecode for the toplevel block */
  ReturnInst::Create(getGlobalContext(), bblock);
  popBlock();

  /* Print the bytecode in a human-readable format
     to see if our program compiled properly
   */
  std::cout << "Code is generated.\n";
  PassManager pm;
  outs() << *module << "\n";
  /// pm.add(createPrintModulePass(outs()));
  pm.run(*module);
}

/* Executes the AST by running the main function */
GenericValue CodeGenContext::runCode() {
  std::cout << "Running code...\n";
#if LLVM_VERSION_CODE >= LLVM_VERSION(3, 5)
  EngineBuilder builder(std::move(module));
#else
  EngineBuilder builder(module.get());
#endif
  builder.setEngineKind(EngineKind::Interpreter);
  ExecutionEngine* ee = builder.create();
  std::vector<GenericValue> noargs;
  GenericValue v = ee->runFunction(mainFunction, noargs);
  std::cout << "Code was run.\n";
  return v;
}

/* Returns an LLVM type based on the identifier */
static Type* typeOf(const NIdentifier& type) {
  if (type.name.compare("int") == 0) {
    return Type::getInt64Ty(getGlobalContext());
  } else if (type.name.compare("double") == 0) {
    return Type::getDoubleTy(getGlobalContext());
  }
  return Type::getVoidTy(getGlobalContext());
}

/* -- Code Generation -- */

Value* NInteger::codeGen(CodeGenContext& context) {
  std::cout << "Creating integer: " << value << std::endl;
  return ConstantInt::get(Type::getInt64Ty(getGlobalContext()), value, true);
}

Value* NDouble::codeGen(CodeGenContext& context) {
  std::cout << "Creating double: " << value << std::endl;
  return ConstantFP::get(Type::getDoubleTy(getGlobalContext()), value);
}

Value* NIdentifier::codeGen(CodeGenContext& context) {
  std::cout << "Creating identifier reference: " << name << std::endl;
  if (context.locals().find(name) == context.locals().end()) {
    std::cerr << "undeclared variable " << name << std::endl;
    return NULL;
  }
  return new LoadInst(context.locals()[name], "", false,
                      context.currentBlock());
}

Value* NMethodCall::codeGen(CodeGenContext& context) {
  Function* function = context.module->getFunction(id.name.c_str());
  if (function == NULL) {
    std::cerr << "no such function " << id.name << std::endl;
  }
  std::vector<Value*> args;
  ExpressionList::const_iterator it;
  for (it = arguments.begin(); it != arguments.end(); it++) {
    args.push_back((**it).codeGen(context));
  }
  CallInst* call = CallInst::Create(function, makeArrayRef(args), "",
                                    context.currentBlock());
  std::cout << "Creating method call: " << id.name << std::endl;
  return call;
}

Value* NBinaryOperator::codeGen(CodeGenContext& context) {
  std::cout << "Creating binary operation " << op << std::endl;
  Instruction::BinaryOps instr;
  switch (op) {
    case TPLUS:
      instr = Instruction::Add;
      goto math;
    case TMINUS:
      instr = Instruction::Sub;
      goto math;
    case TMUL:
      instr = Instruction::Mul;
      goto math;
    case TDIV:
      instr = Instruction::SDiv;
      goto math;

      /* TODO comparison */
  }

  return NULL;
math:
  return BinaryOperator::Create(instr, lhs.codeGen(context),
                                rhs.codeGen(context), "",
                                context.currentBlock());
}

Value* NAssignment::codeGen(CodeGenContext& context) {
  std::cout << "Creating assignment for " << lhs.name << std::endl;
  if (context.locals().find(lhs.name) == context.locals().end()) {
    std::cerr << "undeclared variable " << lhs.name << std::endl;
    return NULL;
  }
  return new StoreInst(rhs.codeGen(context), context.locals()[lhs.name], false,
                       context.currentBlock());
}

Value* NBlock::codeGen(CodeGenContext& context) {
  StatementList::const_iterator it;
  Value* last = NULL;
  for (it = statements.begin(); it != statements.end(); it++) {
    /// std::cout << "Generating code for " << typeid(**it).name() << endl;
    last = (**it).codeGen(context);
  }
  std::cout << "Creating block" << std::endl;
  return last;
}

Value* NExpressionStatement::codeGen(CodeGenContext& context) {
  /// std::cout << "Generating code for " << typeid(expression).name() << endl;
  return expression.codeGen(context);
}

Value* NReturnStatement::codeGen(CodeGenContext& context) {
  /// std::cout << "Generating return code for " << typeid(expression).name()
  ///           << endl;
  Value* returnValue = expression.codeGen(context);
  context.setCurrentReturnValue(returnValue);
  return returnValue;
}

Value* NVariableDeclaration::codeGen(CodeGenContext& context) {
  std::cout << "Creating variable declaration " << type.name << " " << id.name
            << std::endl;
  AllocaInst* alloc =
      new AllocaInst(typeOf(type), id.name.c_str(), context.currentBlock());
  context.locals()[id.name] = alloc;
  if (assignmentExpr != NULL) {
    NAssignment assn(id, *assignmentExpr);
    assn.codeGen(context);
  }
  return alloc;
}

Value* NFunctionDeclaration::codeGen(CodeGenContext& context) {
  std::vector<Type*> argTypes;
  VariableList::const_iterator it;
  for (it = arguments.begin(); it != arguments.end(); it++) {
    argTypes.push_back(typeOf((**it).type));
  }
  FunctionType* ftype =
      FunctionType::get(typeOf(type), makeArrayRef(argTypes), false);
  Function* function = Function::Create(ftype, GlobalValue::InternalLinkage,
                                        id.name.c_str(), context.module.get());
  BasicBlock* bblock =
      BasicBlock::Create(getGlobalContext(), "entry", function, 0);

  context.pushBlock(bblock);

  Function::arg_iterator argsValues = function->arg_begin();
  Value* argumentValue;

  for (it = arguments.begin(); it != arguments.end(); it++) {
    (**it).codeGen(context);

    argumentValue = argsValues++;
    argumentValue->setName((*it)->id.name.c_str());
    StoreInst* inst = new StoreInst(
        argumentValue, context.locals()[(*it)->id.name], false, bblock);
  }

  block.codeGen(context);
  ReturnInst::Create(getGlobalContext(), context.getCurrentReturnValue(),
                     bblock);

  context.popBlock();
  std::cout << "Creating function: " << id.name << std::endl;
  return function;
}
