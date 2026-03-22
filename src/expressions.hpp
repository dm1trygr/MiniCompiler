#pragma once

#include <memory>

#include "context.hpp"

class Visitor;

class Expression {
 public:
  virtual ~Expression() = default;
  virtual void accept(Visitor& visitor) = 0;
};

void printIndent(int depth);

class NumberExpression : public Expression {
 public:
  NumberExpression(int val) : value(val) {}
  void accept(Visitor& visitor) override;

 public:
  int value;
};

class VarExpression : public Expression {
 public:
  VarExpression(const std::string& n) : name(n) {}
  void accept(Visitor& visitor) override;

 public:
  std::string name;
};

class BinaryExpression : public Expression {
 public:
  BinaryExpression(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r,
                   const std::string& o)
      : left(std::move(l)), right(std::move(r)), op(o) {}

  void accept(Visitor& visitor) override;

 public:
  std::unique_ptr<Expression> left, right;
  std::string op;
};