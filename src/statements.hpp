#pragma once

#include <string>
#include <vector>

#include "expressions.hpp"
#include "types.hpp"
#include "utils.hpp"

class Visitor;

class Statement {
 public:
  virtual ~Statement() = default;
  virtual void Accept(Visitor& visitor) = 0;
};

class DeclareStatement : public Statement {
 public:
  DeclareStatement(const std::string& n, const Type& t) : name(n), type(t) {}
  void Accept(Visitor& visitor) override;

 public:
  std::string name;
  Type type;
};

class AssignStatement : public Statement {
 public:
  AssignStatement(const std::string& n, std::unique_ptr<Expression> e)
      : name(n), expr(std::move(e)) {}
  void Accept(Visitor& visitor) override;

 public:
  std::string name;
  std::unique_ptr<Expression> expr;
};

class PrintStatement : public Statement {
 public:
  explicit PrintStatement(std::unique_ptr<Expression> e) : expr(std::move(e)) {}
  void Accept(Visitor& visitor) override;

 public:
  std::unique_ptr<Expression> expr;
};

class BlockStatement : public Statement {
 public:
  void Accept(Visitor& visitor) override;

 public:
  std::vector<std::unique_ptr<Statement>> statements;
};

class IfStatement : public Statement {
 public:
  IfStatement(std::unique_ptr<Expression> cond,
              std::unique_ptr<Statement> then_b,
              std::unique_ptr<Statement> else_b)
      : condition(std::move(cond)),
        then_branch(std::move(then_b)),
        else_branch(std::move(else_b)) {}

  void Accept(Visitor& visitor) override;

 public:
  std::unique_ptr<Expression> condition;
  std::unique_ptr<Statement> then_branch;
  std::unique_ptr<Statement> else_branch;
};

class WhileStatement : public Statement {
 public:
  WhileStatement(std::unique_ptr<Expression> cond, std::unique_ptr<Statement> b)
      : condition(std::move(cond)), body(std::move(b)) {}

  void Accept(Visitor& visitor) override;

 public:
  std::unique_ptr<Expression> condition;
  std::unique_ptr<Statement> body;
};

class ReturnStatement : public Statement {
 public:
  explicit ReturnStatement(std::unique_ptr<Expression> e)
      : expr(std::move(e)) {}
  void Accept(Visitor& visitor) override;

 public:
  std::unique_ptr<Expression> expr;
};

class MethodDeclarationStatement : public Statement {
 public:
  MethodDeclarationStatement(const std::string& n, const Type& ret,
                             std::vector<VariableInfo> args,
                             std::unique_ptr<BlockStatement> b)
      : data(n, ret, std::move(args), std::move(b)) {}
  void Accept(Visitor& visitor) override;

 public:
  MethodInfo data;
};

class ClassDeclarationStatement : public Statement {
 public:
  explicit ClassDeclarationStatement(const std::string& n) : name(n) {}
  void Accept(Visitor& visitor) override;

 public:
  std::string name;
  std::vector<std::unique_ptr<DeclareStatement>> fields;
  std::vector<std::unique_ptr<MethodDeclarationStatement>> methods;
};
