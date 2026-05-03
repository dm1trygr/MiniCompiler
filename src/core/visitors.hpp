#pragma once

#include <fstream>
#include <unordered_map>

#include "expressions.hpp"
#include "statements.hpp"
#include "symtable.hpp"
#include "types.hpp"

class Visitor {
 public:
  virtual ~Visitor() = default;

  virtual void Visit(NumberExpression* node) = 0;
  virtual void Visit(VarExpression* node) = 0;
  virtual void Visit(BinaryExpression* node) = 0;
  virtual void Visit(FieldAccessExpression* node) = 0;
  virtual void Visit(MethodCallExpression* node) = 0;
  virtual void Visit(FunctionCallExpression* node) = 0;

  virtual void Visit(DeclareStatement* node) = 0;
  virtual void Visit(AssignStatement* node) = 0;
  virtual void Visit(FieldAssignStatement* node) = 0;
  virtual void Visit(ExpressionStatement* node) = 0;
  virtual void Visit(PrintStatement* node) = 0;
  virtual void Visit(BlockStatement* node) = 0;
  virtual void Visit(IfStatement* node) = 0;
  virtual void Visit(WhileStatement* node) = 0;

  virtual void Visit(ClassDeclarationStatement* node) = 0;
  virtual void Visit(MethodDeclarationStatement* node) = 0;
  virtual void Visit(ReturnStatement* node) = 0;
};

class PrintVisitor : public Visitor {
 public:
  explicit PrintVisitor(const std::string& filename);
  ~PrintVisitor() override;

  void Visit(NumberExpression* node) override;
  void Visit(VarExpression* node) override;
  void Visit(BinaryExpression* node) override;
  void Visit(FieldAccessExpression* node) override;
  void Visit(MethodCallExpression* node) override;
  void Visit(FunctionCallExpression* node) override;

  void Visit(DeclareStatement* node) override;
  void Visit(AssignStatement* node) override;
  void Visit(FieldAssignStatement* node) override;
  void Visit(ExpressionStatement* node) override;
  void Visit(PrintStatement* node) override;
  void Visit(BlockStatement* node) override;
  void Visit(IfStatement* node) override;
  void Visit(WhileStatement* node) override;

  void Visit(ClassDeclarationStatement* node) override;
  void Visit(MethodDeclarationStatement* node) override;
  void Visit(ReturnStatement* node) override;

 private:
  std::ofstream out;
  int depth = 0;

  void PrintIndent();
};

class Interpreter : public Visitor {
 public:
  void Visit(NumberExpression* node) override;
  void Visit(VarExpression* node) override;
  void Visit(BinaryExpression* node) override;
  void Visit(FieldAccessExpression* node) override;
  void Visit(MethodCallExpression* node) override;
  void Visit(FunctionCallExpression* node) override;

  void Visit(DeclareStatement* node) override;
  void Visit(AssignStatement* node) override;
  void Visit(FieldAssignStatement* node) override;
  void Visit(ExpressionStatement* node) override;
  void Visit(PrintStatement* node) override;
  void Visit(BlockStatement* node) override;
  void Visit(IfStatement* node) override;
  void Visit(WhileStatement* node) override;

  void Visit(ClassDeclarationStatement* node) override;
  void Visit(MethodDeclarationStatement* node) override;
  void Visit(ReturnStatement* node) override;

 private:
  std::unordered_map<std::string, int> variables;
  int result_value = 0;
};
