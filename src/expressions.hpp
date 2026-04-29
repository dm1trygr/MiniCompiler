#pragma once

#include <memory>
#include <string>

#include "operators.hpp"

class Visitor;

class Expression {
 public:
  virtual ~Expression() = default;
  virtual void Accept(Visitor& visitor) = 0;
};

class NumberExpression : public Expression {
 public:
  explicit NumberExpression(int val) : value(val) {}
  void Accept(Visitor& visitor) override;

  int value;
};

class VarExpression : public Expression {
 public:
  explicit VarExpression(const std::string& n) : name(n) {}
  void Accept(Visitor& visitor) override;

  std::string name;
};

class BinaryExpression : public Expression {
 public:
  BinaryExpression(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r,
                   BinaryOperator o)
      : left(std::move(l)), right(std::move(r)), op(o) {}
  void Accept(Visitor& visitor) override;

  std::unique_ptr<Expression> left, right;
  BinaryOperator op;
};
