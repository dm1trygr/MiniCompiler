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
  out << "BinaryOp [" << node->op << "]\n";
  depth++;
  node->left->Accept(*this);
  node->right->Accept(*this);
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
  out << "Method: " << node->name << "(";
  for (size_t i = 0; i < node->arguments.size(); ++i) {
    out << node->arguments[i].first << ": "
        << node->arguments[i].second.ToString();
    if (i < node->arguments.size() - 1) out << ", ";
  }
  out << ") -> " << node->return_type.ToString() << "\n";

  depth++;
  node->body->Accept(*this);
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
    throw std::runtime_error("Variable not declared: " + node->name);
  }
  result_value = variables[node->name];
}

void Interpreter::Visit(BinaryExpression* node) {
  node->left->Accept(*this);
  int left_val = result_value;

  node->right->Accept(*this);
  int right_val = result_value;

  if (node->op == "==") {
    result_value = (left_val == right_val) ? 1 : 0;
  } else if (node->op == "+") {
    result_value = left_val + right_val;
  } else if (node->op == "-") {
    result_value = left_val - right_val;
  } else if (node->op == "*") {
    result_value = left_val * right_val;
  } else if (node->op == "/") {
    if (right_val == 0) {
      throw std::runtime_error("Division by zero");
    }
    result_value = left_val / right_val;
  } else {
    throw std::runtime_error("Unknown operator: " + node->op);
  }
}

void Interpreter::Visit(DeclareStatement* node) { variables[node->name] = 0; }

void Interpreter::Visit(AssignStatement* node) {
  if (variables.find(node->name) == variables.end()) {
    throw std::runtime_error("Assign of undeclared variable: " + node->name);
  }
  node->expr->Accept(*this);
  variables[node->name] = result_value;
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
  root_scope = std::make_unique<Scope>(nullptr, &global_sym_table);
  current_scope = root_scope.get();
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

void SemanticAnalyzer::Visit(PrintStatement* node) {
  node->expr->Accept(*this);
}

void SemanticAnalyzer::Visit(BlockStatement* node) {
  auto new_scope = std::make_unique<Scope>(current_scope, &global_sym_table);
  Scope* raw_ptr = new_scope.get();
  current_scope->children.push_back(std::move(new_scope));
  current_scope = raw_ptr;

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

  ClassInfo info;
  info.name = node->name;

  for (auto& field : node->fields) {
    if (info.fields.find(field->name) != info.fields.end()) {
      throw std::runtime_error("Field '" + field->name +
                               "' already declared in class '" + node->name +
                               "'");
    }
    info.fields[field->name] = {field->name, field->type};
  }

  for (auto& method : node->methods) {
    if (info.methods.find(method->name) != info.methods.end()) {
      throw std::runtime_error("Method '" + method->name +
                               "' already declared in class '" + node->name +
                               "'");
    }

    MethodInfo m_info;
    m_info.name = method->name;
    m_info.return_type = method->return_type;
    for (auto& arg : method->arguments) {
      m_info.arguments.push_back({arg.first, arg.second});
    }
    info.methods[m_info.name] = m_info;
  }

  global_sym_table.classes[node->name] = info;

  auto class_scope = std::make_unique<Scope>(current_scope, &global_sym_table);
  Scope* class_scope_ptr = class_scope.get();
  current_scope->children.push_back(std::move(class_scope));
  current_scope = class_scope_ptr;

  for (auto& field : node->fields) {
    current_scope->DeclareVariable(field->name, field->type);
  }

  for (auto& method : node->methods) {
    method->Accept(*this);
  }

  current_scope = current_scope->parent;
}

void SemanticAnalyzer::Visit(MethodDeclarationStatement* node) {
  auto method_scope = std::make_unique<Scope>(current_scope, &global_sym_table);
  Scope* raw_ptr = method_scope.get();
  current_scope->children.push_back(std::move(method_scope));
  current_scope = raw_ptr;

  bool prev_in_method = in_method;
  Type prev_return_type = current_return_type;
  in_method = true;
  current_return_type = node->return_type;

  for (auto& arg : node->arguments) {
    if (!current_scope->DeclareVariable(arg.first, arg.second)) {
      throw std::runtime_error("Duplicate argument name '" + arg.first + "'");
    }
  }

  node->body->Accept(*this);

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
