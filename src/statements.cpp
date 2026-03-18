#include "statements.hpp"

#include <iostream>
#include <stdexcept>

void DeclareStatement::execute(Context& ctx) { ctx.variables[name] = 0; }

void DeclareStatement::printTree(int depth) {
  printIndent(depth);
  std::cout << "Declare: " << name << " : int\n";
}

void AssignStatement::execute(Context& ctx) {
  if (ctx.variables.find(name) == ctx.variables.end())
    throw std::runtime_error("Assign of undeclared variable: " + name);
  ctx.variables[name] = expr->evaluate(ctx);
}

void AssignStatement::printTree(int depth) {
  printIndent(depth);
  std::cout << "Assign: " << name << " =\n";
  expr->printTree(depth + 1);
}

void PrintStatement::execute(Context& ctx) {
  std::cout << ">>> " << expr->evaluate(ctx) << "\n";
}

void PrintStatement::printTree(int depth) {
  printIndent(depth);
  std::cout << "Print:\n";
  expr->printTree(depth + 1);
}

void BlockStatement::execute(Context& ctx) {
  for (auto& stmt : statements) stmt->execute(ctx);
}

void BlockStatement::printTree(int depth) {
  printIndent(depth);
  std::cout << "Block:\n";
  for (auto& stmt : statements) stmt->printTree(depth + 1);
}

void IfStatement::execute(Context& ctx) {
  if (condition->evaluate(ctx)) {
    thenBranch->execute(ctx);
  } else if (elseBranch) {
    elseBranch->execute(ctx);
  }
}

void IfStatement::printTree(int depth) {
  printIndent(depth);
  std::cout << "If:\n";
  printIndent(depth + 1);
  std::cout << "Condition:\n";
  condition->printTree(depth + 2);
  printIndent(depth + 1);
  std::cout << "Then:\n";
  thenBranch->printTree(depth + 2);
  if (elseBranch) {
    printIndent(depth + 1);
    std::cout << "Else:\n";
    elseBranch->printTree(depth + 2);
  }
}