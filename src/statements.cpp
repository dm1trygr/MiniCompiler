#include "statements.hpp"

#include <iostream>
#include <stdexcept>

#include "visitors.hpp"

void DeclareStatement::accept(Visitor& v) { v.visit(this); }

void AssignStatement::accept(Visitor& v) { v.visit(this); }

void PrintStatement::accept(Visitor& v) { v.visit(this); }

void BlockStatement::accept(Visitor& v) { v.visit(this); }

void IfStatement::accept(Visitor& v) { v.visit(this); }
