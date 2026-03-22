#pragma once

#include <string>
#include <vector>

#include "expressions.hpp"

class Visitor;

class Statement {
 public:
  virtual ~Statement() = default;
  virtual void accept(Visitor& visitor) = 0;
};

class DeclareStatement : public Statement {
 public:
  DeclareStatement(const std::string& n) : name(n) {}
  void accept(Visitor& visitor) override;

 public:
  std::string name;
};

class AssignStatement : public Statement {
 public:
  AssignStatement(const std::string& n, std::unique_ptr<Expression> e)
      : name(n), expr(std::move(e)) {}
  void accept(Visitor& visitor) override;

 public:
  std::string name;
  std::unique_ptr<Expression> expr;
};

class PrintStatement : public Statement {
 public:
  PrintStatement(std::unique_ptr<Expression> e) : expr(std::move(e)) {}
  void accept(Visitor& visitor) override;

 public:
  std::unique_ptr<Expression> expr;
};

class BlockStatement : public Statement {
 public:
  void accept(Visitor& visitor) override;

 public:
  std::vector<std::unique_ptr<Statement>> statements;
};

class IfStatement : public Statement {
 public:
  IfStatement(std::unique_ptr<Expression> cond,
              std::unique_ptr<Statement> thenB,
              std::unique_ptr<Statement> elseB)
      : condition(std::move(cond)),
        thenBranch(std::move(thenB)),
        elseBranch(std::move(elseB)) {}

  void accept(Visitor& visitor) override;

 public:
  std::unique_ptr<Expression> condition;
  std::unique_ptr<Statement> thenBranch;
  std::unique_ptr<Statement> elseBranch;
};
