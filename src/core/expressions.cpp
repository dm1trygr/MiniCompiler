#include "expressions.hpp"

#include "visitors.hpp"

void NumberExpression::Accept(Visitor& v) { v.Visit(this); }

void VarExpression::Accept(Visitor& v) { v.Visit(this); }

void BinaryExpression::Accept(Visitor& v) { v.Visit(this); }

void FieldAccessExpression::Accept(Visitor& v) { v.Visit(this); }

void MethodCallExpression::Accept(Visitor& v) { v.Visit(this); }

void FunctionCallExpression::Accept(Visitor& v) { v.Visit(this); }
