#pragma once

#include <memory>

#include "context.hpp"

class Expression {
 public:
  virtual ~Expression() = default;
  virtual int evaluate(Context& ctx) = 0;
  virtual void printTree(int depth) = 0;
};

void printIndent(int depth);

class NumberExpression : public Expression {
 public:
  NumberExpression(int val) : value(val) {}
  int evaluate(Context& ctx) override;
  void printTree(int depth) override;

 private:
  int value;
};

class VarExpression : public Expression {
 public:
  VarExpression(const std::string& n) : name(n) {}
  int evaluate(Context& ctx) override;
  void printTree(int depth) override;

 private:
  std::string name;
};

class BinaryExpression : public Expression {
 public:
  BinaryExpression(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r,
                   const std::string& o)
      : left(std::move(l)), right(std::move(r)), op(o) {}

  int evaluate(Context& ctx) override;
  void printTree(int depth) override;

 private:
  std::unique_ptr<Expression> left, right;
  std::string op;
};