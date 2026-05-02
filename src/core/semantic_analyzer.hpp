#pragma once

#include <memory>
#include <string>

#include "expressions.hpp"
#include "statements.hpp"
#include "symtable.hpp"
#include "types.hpp"
#include "visitors.hpp"

class SemanticAnalyzer : public Visitor {
 public:
  SemanticAnalyzer();

  Scope* GetRootScope() { return root_scope.get(); }
  GlobalSymbolTable& GetGlobalSymTable() { return global_sym_table; }

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
  GlobalSymbolTable global_sym_table;
  std::unique_ptr<Scope> root_scope;
  Scope* current_scope;
  bool in_method = false;
  Type current_return_type;
  Type last_expr_type;
  std::string current_class_name;

  const ClassInfo* ResolveClassOfVar(const std::string& var_name);
  Type GetExpressionType(Expression* expr);
};
