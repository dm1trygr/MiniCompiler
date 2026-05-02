#include "visitors.hpp"

#include <iostream>
#include <stdexcept>

PrintVisitor::PrintVisitor(const std::string& filename) {
  out.open(filename);
  if (!out.is_open()) throw std::runtime_error("Can't open file!");
}

PrintVisitor::~PrintVisitor() {
  if (out.is_open()) out.close();
}

void PrintVisitor::Visit(NumberExpression* node) {
  PrintIndent();
  out << "Number: " << node->value << "\n";
}

void PrintVisitor::Visit(VarExpression* node) {
  PrintIndent();
  out << "Variable: " << node->name << "\n";
}

void PrintVisitor::Visit(BinaryExpression* node) {
  PrintIndent();
  out << "BinaryOp [" << OperatorToString(node->op) << "]\n";
  depth++;
  node->left->Accept(*this);
  node->right->Accept(*this);
  depth--;
}

void PrintVisitor::Visit(FieldAccessExpression* node) {
  PrintIndent();
  out << "FieldAccess: " << node->object_name << "." << node->field_name << "\n";
}

void PrintVisitor::Visit(MethodCallExpression* node) {
  PrintIndent();
  out << "MethodCall: " << node->object_name << "." << node->method_name << "()\n";
  depth++;
  for (auto& arg : node->arguments) {
    arg->Accept(*this);
  }
  depth--;
}

void PrintVisitor::Visit(DeclareStatement* node) {
  PrintIndent();
  out << "Declare: " << node->name << " : " << node->type.ToString() << "\n";
}

void PrintVisitor::Visit(AssignStatement* node) {
  PrintIndent();
  out << "Assign: " << node->name << " =\n";
  depth++;
  node->expr->Accept(*this);
  depth--;
}

void PrintVisitor::Visit(FieldAssignStatement* node) {
  PrintIndent();
  out << "FieldAssign: " << node->object_name << "." << node->field_name << " =\n";
  depth++;
  node->expr->Accept(*this);
  depth--;
}

void PrintVisitor::Visit(ExpressionStatement* node) {
  PrintIndent();
  out << "ExpressionStatement:\n";
  depth++;
  node->expr->Accept(*this);
  depth--;
}

void PrintVisitor::Visit(PrintStatement* node) {
  PrintIndent();
  out << "Print:\n";
  depth++;
  node->expr->Accept(*this);
  depth--;
}

void PrintVisitor::Visit(BlockStatement* node) {
  PrintIndent();
  out << "Block:\n";
  depth++;
  for (auto& stmt : node->statements) {
    stmt->Accept(*this);
  }
  depth--;
}

void PrintVisitor::Visit(IfStatement* node) {
  PrintIndent();
  out << "If:\n";
  depth++;
  PrintIndent();
  out << "Condition:\n";
  depth++;
  node->condition->Accept(*this);
  depth--;

  PrintIndent();
  out << "Then:\n";
  depth++;
  node->then_branch->Accept(*this);
  depth--;

  if (node->else_branch) {
    PrintIndent();
    out << "Else:\n";
    depth++;
    node->else_branch->Accept(*this);
    depth--;
  }
  depth--;
}

void PrintVisitor::Visit(WhileStatement* node) {
  PrintIndent();
  out << "While:\n";
  depth++;
  PrintIndent();
  out << "Condition:\n";
  depth++;
  node->condition->Accept(*this);
  depth--;

  PrintIndent();
  out << "Body:\n";
  depth++;
  node->body->Accept(*this);
  depth--;
  depth--;
}

void PrintVisitor::Visit(ClassDeclarationStatement* node) {
  PrintIndent();
  out << "Class: " << node->name << "\n";
  depth++;

  if (!node->fields.empty()) {
    PrintIndent();
    out << "Fields:\n";
    depth++;
    for (auto& field : node->fields) {
      field->Accept(*this);
    }
    depth--;
  }

  if (!node->methods.empty()) {
    PrintIndent();
    out << "Methods:\n";
    depth++;
    for (auto& method : node->methods) {
      method->Accept(*this);
    }
    depth--;
  }

  depth--;
}

void PrintVisitor::Visit(MethodDeclarationStatement* node) {
  PrintIndent();
  out << "Method: " << node->data.name << "(";
  for (size_t i = 0; i < node->data.arguments.size(); ++i) {
    out << node->data.arguments[i].name << ": "
        << node->data.arguments[i].type.ToString();
    if (i < node->data.arguments.size() - 1) out << ", ";
  }
  out << ") -> " << node->data.return_type.ToString() << "\n";

  depth++;
  node->data.body->Accept(*this);
  depth--;
}

void PrintVisitor::Visit(ReturnStatement* node) {
  PrintIndent();
  out << "Return:";
  if (node->expr) {
    out << "\n";
    depth++;
    node->expr->Accept(*this);
    depth--;
  } else {
    out << " void\n";
  }
}

void PrintVisitor::PrintIndent() {
  for (int i = 0; i < depth; ++i) out << "  ";
}

void Interpreter::Visit(NumberExpression* node) { result_value = node->value; }

void Interpreter::Visit(VarExpression* node) {
  if (variables.find(node->name) == variables.end()) {
    throw std::runtime_error("Variable '" + node->name + "' not declared");
  }
  result_value = variables[node->name];
}

void Interpreter::Visit(BinaryExpression* node) {
  node->left->Accept(*this);
  int left_val = result_value;

  node->right->Accept(*this);
  int right_val = result_value;

  switch (node->op) {
    case BinaryOperator::EQUAL:
      result_value = (left_val == right_val) ? 1 : 0;
      break;
    case BinaryOperator::PLUS:
      result_value = left_val + right_val;
      break;
    case BinaryOperator::MINUS:
      result_value = left_val - right_val;
      break;
    case BinaryOperator::MULTIPLY:
      result_value = left_val * right_val;
      break;
    case BinaryOperator::DIVIDE:
      if (right_val == 0) {
        throw std::runtime_error("Division by zero");
      }
      result_value = left_val / right_val;
      break;
  }
}

void Interpreter::Visit(FieldAccessExpression*) {}

void Interpreter::Visit(MethodCallExpression*) {}

void Interpreter::Visit(DeclareStatement* node) { variables[node->name] = 0; }

void Interpreter::Visit(AssignStatement* node) {
  if (variables.find(node->name) == variables.end()) {
    throw std::runtime_error("Assignment to undeclared variable '" +
                             node->name + "'");
  }
  node->expr->Accept(*this);
  variables[node->name] = result_value;
}

void Interpreter::Visit(FieldAssignStatement*) {}

void Interpreter::Visit(ExpressionStatement* node) {
  node->expr->Accept(*this);
}

void Interpreter::Visit(PrintStatement* node) {
  node->expr->Accept(*this);
  std::cout << result_value << std::endl;
}

void Interpreter::Visit(BlockStatement* node) {
  for (auto& stmt : node->statements) {
    stmt->Accept(*this);
  }
}

void Interpreter::Visit(IfStatement* node) {
  node->condition->Accept(*this);
  if (result_value != 0) {
    node->then_branch->Accept(*this);
  } else if (node->else_branch) {
    node->else_branch->Accept(*this);
  }
}

void Interpreter::Visit(WhileStatement* node) {
  while (true) {
    node->condition->Accept(*this);
    if (result_value == 0) break;
    node->body->Accept(*this);
  }
}

void Interpreter::Visit(ClassDeclarationStatement*) {}

void Interpreter::Visit(MethodDeclarationStatement*) {}

void Interpreter::Visit(ReturnStatement*) {}

SemanticAnalyzer::SemanticAnalyzer() {
  root_scope = std::make_unique<Scope>(nullptr, global_sym_table);
  current_scope = root_scope.get();
}

const ClassInfo* SemanticAnalyzer::ResolveClassOfVar(const std::string& var_name) {
  VariableInfo* var = current_scope->ResolveVariable(var_name);
  if (!var) {
    throw std::runtime_error("Undeclared variable '" + var_name + "'");
  }
  auto it = global_sym_table.classes.find(var->type.class_name);
  if (it == global_sym_table.classes.end()) {
    throw std::runtime_error("Variable '" + var_name + "' is not of class type");
  }
  return &it->second;
}

void SemanticAnalyzer::Visit(NumberExpression*) {}

void SemanticAnalyzer::Visit(VarExpression* node) {
  VariableInfo* var = current_scope->ResolveVariable(node->name);
  if (!var) {
    throw std::runtime_error("Use of undeclared variable '" + node->name + "'");
  }
}

void SemanticAnalyzer::Visit(BinaryExpression* node) {
  node->left->Accept(*this);
  node->right->Accept(*this);
}

void SemanticAnalyzer::Visit(FieldAccessExpression* node) {
  const ClassInfo* cls = ResolveClassOfVar(node->object_name);
  if (cls->fields.find(node->field_name) == cls->fields.end()) {
    throw std::runtime_error("No field '" + node->field_name + "' in class '" +
                             cls->name + "'");
  }
}

void SemanticAnalyzer::Visit(MethodCallExpression* node) {
  const ClassInfo* cls = ResolveClassOfVar(node->object_name);
  auto it = cls->methods.find(node->method_name);
  if (it == cls->methods.end()) {
    throw std::runtime_error("No method '" + node->method_name + "' in class '" +
                             cls->name + "'");
  }
  if (node->arguments.size() != it->second.arguments.size()) {
    throw std::runtime_error("Wrong number of arguments for method '" +
                             node->method_name + "'");
  }
  for (auto& arg : node->arguments) {
    arg->Accept(*this);
  }
}

void SemanticAnalyzer::Visit(DeclareStatement* node) {
  if (!current_scope->DeclareVariable(node->name, node->type)) {
    throw std::runtime_error("Variable '" + node->name +
                             "' already declared in this scope");
  }
}

void SemanticAnalyzer::Visit(AssignStatement* node) {
  VariableInfo* var = current_scope->ResolveVariable(node->name);
  if (!var) {
    throw std::runtime_error("Assign to undeclared variable '" + node->name +
                             "'");
  }
  node->expr->Accept(*this);
}

void SemanticAnalyzer::Visit(FieldAssignStatement* node) {
  const ClassInfo* cls = ResolveClassOfVar(node->object_name);
  if (cls->fields.find(node->field_name) == cls->fields.end()) {
    throw std::runtime_error("No field '" + node->field_name + "' in class '" +
                             cls->name + "'");
  }
  node->expr->Accept(*this);
}

void SemanticAnalyzer::Visit(ExpressionStatement* node) {
  node->expr->Accept(*this);
}

void SemanticAnalyzer::Visit(PrintStatement* node) {
  node->expr->Accept(*this);
}

void SemanticAnalyzer::Visit(BlockStatement* node) {
  current_scope = current_scope->CreateChildScope();

  for (auto& stmt : node->statements) {
    stmt->Accept(*this);
  }

  current_scope = current_scope->parent;
}

void SemanticAnalyzer::Visit(IfStatement* node) {
  node->condition->Accept(*this);
  node->then_branch->Accept(*this);
  if (node->else_branch) {
    node->else_branch->Accept(*this);
  }
}

void SemanticAnalyzer::Visit(WhileStatement* node) {
  node->condition->Accept(*this);
  node->body->Accept(*this);
}

void SemanticAnalyzer::Visit(ClassDeclarationStatement* node) {
  if (global_sym_table.classes.find(node->name) !=
      global_sym_table.classes.end()) {
    throw std::runtime_error("Class '" + node->name + "' already declared");
  }

  std::unordered_map<std::string, VariableInfo> fields;
  std::unordered_map<std::string, MethodInfo> methods;

  for (auto& field : node->fields) {
    if (fields.find(field->name) != fields.end()) {
      throw std::runtime_error("Field '" + field->name +
                               "' already declared in class '" + node->name +
                               "'");
    }
    fields[field->name] = {field->name, field->type};
  }

  for (auto& method : node->methods) {
    if (methods.find(method->data.name) != methods.end()) {
      throw std::runtime_error("Method '" + method->data.name +
                               "' already declared in class '" + node->name +
                               "'");
    }

    methods.emplace(method->data.name,
                    MethodInfo(method->data.name, method->data.return_type,
                               method->data.arguments));
  }

  global_sym_table.classes.emplace(node->name,
                                   ClassInfo(node->name, std::move(fields),
                                             std::move(methods)));

  current_scope = current_scope->CreateChildScope();

  for (auto& field : node->fields) {
    current_scope->DeclareVariable(field->name, field->type);
  }

  for (auto& method : node->methods) {
    method->Accept(*this);
  }

  current_scope = current_scope->parent;
}

void SemanticAnalyzer::Visit(MethodDeclarationStatement* node) {
  current_scope = current_scope->CreateChildScope();

  bool prev_in_method = in_method;
  Type prev_return_type = current_return_type;
  in_method = true;
  current_return_type = node->data.return_type;

  for (auto& arg : node->data.arguments) {
    if (!current_scope->DeclareVariable(arg.name, arg.type)) {
      throw std::runtime_error("Duplicate argument name '" + arg.name + "'");
    }
  }

  node->data.body->Accept(*this);

  in_method = prev_in_method;
  current_return_type = prev_return_type;
  current_scope = current_scope->parent;
}

void SemanticAnalyzer::Visit(ReturnStatement* node) {
  if (!in_method) {
    throw std::runtime_error("Return statement outside of method");
  }

  if (current_return_type.kind == TypeKind::VOID) {
    if (node->expr) {
      throw std::runtime_error("Void method should not return a value");
    }
  } else {
    if (!node->expr) {
      throw std::runtime_error("Non-void method must return a value");
    }
    node->expr->Accept(*this);
  }
}
