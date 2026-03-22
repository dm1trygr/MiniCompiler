#include "expressions.hpp"

#include <iostream>

#include "visitors.hpp"

void NumberExpression::accept(Visitor& v) { v.visit(this); }

void VarExpression::accept(Visitor& v) { v.visit(this); }

void BinaryExpression::accept(Visitor& v) { v.visit(this); }
