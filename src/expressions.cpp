#include "expressions.hpp"

#include <iostream>

void printIndent(int depth) {
  for (int i = 0; i < depth; ++i) std::cout << "  ";
}

int NumberExpression::evaluate(Context& ctx) { return value; }

void NumberExpression::printTree(int depth) {
  printIndent(depth);
  std::cout << "Number: " << value << "\n";
}

int VarExpression::evaluate(Context& ctx) {
  if (ctx.variables.find(name) == ctx.variables.end())
    throw std::runtime_error("Non-declared variable: " + name);
  return ctx.variables[name];
}

void VarExpression::printTree(int depth) {
  printIndent(depth);
  std::cout << "Variable: " << name << "\n";
}

int BinaryExpression::evaluate(Context& ctx) {
  int lval = left->evaluate(ctx);
  int rval = right->evaluate(ctx);
  if (op == "==") return lval == rval;
  throw std::runtime_error("Unknown operator: " + op);
}

void BinaryExpression::printTree(int depth) {
  printIndent(depth);
  std::cout << "BinaryOp [" << op << "]\n";
  left->printTree(depth + 1);
  right->printTree(depth + 1);
}
