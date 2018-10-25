#include <iostream>
#include "codegen.h"
#include "node.h"

extern int yyparse();
extern NBlock* programBlock;

void createCoreFunctions(CodeGenContext& context);

int main(int argc, char** argv) {
  yyparse();
  std::cout << programBlock << std::endl;
  InitializeNativeTarget();
  CodeGenContext context;
  createCoreFunctions(context);
  context.generateCode(*programBlock);
  context.runCode();

  return 0;
}
