#pragma once

#include <memory>
#include <string>
#include <vector>

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

class FieldAccessExpression : public Expression {
 public:
  FieldAccessExpression(const std::string& obj, const std::string& field)
      : object_name(obj), field_name(field) {}
  void Accept(Visitor& visitor) override;

 public:
  std::string object_name;
  std::string field_name;
};

class MethodCallExpression : public Expression {
 public:
  MethodCallExpression(const std::string& obj, const std::string& method,
                       std::vector<std::unique_ptr<Expression>> args)
      : object_name(obj), method_name(method), arguments(std::move(args)) {}
  void Accept(Visitor& visitor) override;

 public:
  std::string object_name;
  std::string method_name;
  std::vector<std::unique_ptr<Expression>> arguments;
};

class FunctionCallExpression : public Expression {
 public:
  FunctionCallExpression(const std::string& func,
                         std::vector<std::unique_ptr<Expression>> args)
      : function_name(func), arguments(std::move(args)) {}
  void Accept(Visitor& visitor) override;

 public:
  std::string function_name;
  std::vector<std::unique_ptr<Expression>> arguments;
};
