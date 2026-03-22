#pragma once

#include <fstream>

#include "expressions.hpp"
#include "statements.hpp"

class Visitor {
 public:
  virtual ~Visitor() = default;

  virtual void visit(NumberExpression* node) = 0;
  virtual void visit(VarExpression* node) = 0;
  virtual void visit(BinaryExpression* node) = 0;

  virtual void visit(DeclareStatement* node) = 0;
  virtual void visit(AssignStatement* node) = 0;
  virtual void visit(PrintStatement* node) = 0;
  virtual void visit(BlockStatement* node) = 0;
  virtual void visit(IfStatement* node) = 0;
};

class PrintVisitor : public Visitor {
 public:
  PrintVisitor(const std::string& filename);
  ~PrintVisitor();

  void visit(NumberExpression* node) override;
  void visit(VarExpression* node) override;
  void visit(BinaryExpression* node) override;

  void visit(DeclareStatement* node) override;
  void visit(AssignStatement* node) override;
  void visit(PrintStatement* node) override;
  void visit(BlockStatement* node) override;
  void visit(IfStatement* node) override;

 private:
  std::ofstream out;
  int depth = 0;

  void printIndent();
};

class Interpreter : public Visitor {
 public:
  void visit(NumberExpression* node) override;
  void visit(VarExpression* node) override;
  void visit(BinaryExpression* node) override;

  void visit(DeclareStatement* node) override;
  void visit(AssignStatement* node) override;
  void visit(PrintStatement* node) override;
  void visit(BlockStatement* node) override;
  void visit(IfStatement* node) override;

 private:
  Context ctx;
  int result_value = 0;
};