#include "visitors.hpp"

#include <iostream>
#include <stdexcept>

PrintVisitor::PrintVisitor(const std::string& filename) {
  out.open(filename);
  if (!out.is_open()) throw std::runtime_error("Can't open file!");
}

PrintVisitor::~PrintVisitor() {
  if (out.is_open()) out.close();
}

void PrintVisitor::visit(NumberExpression* node) {
  printIndent();
  out << "Number: " << node->value << "\n";
}

void PrintVisitor::visit(VarExpression* node) {
  printIndent();
  out << "Variable: " << node->name << "\n";
}

void PrintVisitor::visit(BinaryExpression* node) {
  printIndent();
  out << "BinaryOp [" << node->op << "]\n";
  depth++;
  node->left->accept(*this);
  node->right->accept(*this);
  depth--;
}

void PrintVisitor::visit(DeclareStatement* node) {
  printIndent();
  out << "Declare: " << node->name << " : int\n";
}

void PrintVisitor::visit(AssignStatement* node) {
  printIndent();
  out << "Assign: " << node->name << " =\n";
  depth++;
  node->expr->accept(*this);
  depth--;
}

void PrintVisitor::visit(PrintStatement* node) {
  printIndent();
  out << "Print:\n";
  depth++;
  node->expr->accept(*this);
  depth--;
}

void PrintVisitor::visit(BlockStatement* node) {
  printIndent();
  out << "Block:\n";
  depth++;
  for (auto& stmt : node->statements) {
    stmt->accept(*this);
  }
  depth--;
}

void PrintVisitor::visit(IfStatement* node) {
  printIndent();
  out << "If:\n";
  depth++;
  printIndent();
  out << "Condition:\n";
  depth++;
  node->condition->accept(*this);
  depth--;

  printIndent();
  out << "Then:\n";
  depth++;
  node->thenBranch->accept(*this);
  depth--;

  if (node->elseBranch) {
    printIndent();
    out << "Else:\n";
    depth++;
    node->elseBranch->accept(*this);
    depth--;
  }
  depth--;
}

void PrintVisitor::printIndent() {
  for (int i = 0; i < depth; ++i) out << "  ";
}

void Interpreter::visit(NumberExpression* node) { result_value = node->value; }

void Interpreter::visit(VarExpression* node) {
  if (ctx.variables.find(node->name) == ctx.variables.end()) {
    throw std::runtime_error("Переменная не объявлена: " + node->name);
  }
  result_value = ctx.variables[node->name];
}

void Interpreter::visit(BinaryExpression* node) {
  node->left->accept(*this);
  int left_val = result_value;

  node->right->accept(*this);
  int right_val = result_value;

  if (node->op == "==")
    result_value = (left_val == right_val);
  else if (node->op == "!=")
    result_value = (left_val != right_val);
  else if (node->op == "<")
    result_value = (left_val < right_val);
  else if (node->op == "+")
    result_value = (left_val + right_val);
  else if (node->op == "-")
    result_value = (left_val - right_val);
  else
    throw std::runtime_error("Неизвестный оператор: " + node->op);
}

void Interpreter::visit(DeclareStatement* node) {
  ctx.variables[node->name] = 0;
}

void Interpreter::visit(AssignStatement* node) {
  if (ctx.variables.find(node->name) == ctx.variables.end()) {
    throw std::runtime_error("Присваивание необъявленной переменной: " +
                             node->name);
  }
  node->expr->accept(*this);
  ctx.variables[node->name] = result_value;
}

void Interpreter::visit(PrintStatement* node) {
  node->expr->accept(*this);
  std::cout << result_value << std::endl;
}

void Interpreter::visit(BlockStatement* node) {
  for (auto& stmt : node->statements) {
    stmt->accept(*this);
  }
}

void Interpreter::visit(IfStatement* node) {
  node->condition->accept(*this);
  if (result_value != 0) {
    node->thenBranch->accept(*this);
  } else if (node->elseBranch) {
    node->elseBranch->accept(*this);
  }
}
