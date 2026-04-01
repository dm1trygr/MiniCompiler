#include "statements.hpp"

#include "visitors.hpp"

void DeclareStatement::Accept(Visitor& v) { v.Visit(this); }

void AssignStatement::Accept(Visitor& v) { v.Visit(this); }

void PrintStatement::Accept(Visitor& v) { v.Visit(this); }

void BlockStatement::Accept(Visitor& v) { v.Visit(this); }

void IfStatement::Accept(Visitor& v) { v.Visit(this); }

void WhileStatement::Accept(Visitor& v) { v.Visit(this); }

void ReturnStatement::Accept(Visitor& v) { v.Visit(this); }

void MethodDeclarationStatement::Accept(Visitor& v) { v.Visit(this); }

void ClassDeclarationStatement::Accept(Visitor& v) { v.Visit(this); }
