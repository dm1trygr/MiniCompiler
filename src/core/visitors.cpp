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

void PrintVisitor::Visit(FunctionCallExpression* node) {
  PrintIndent();
  out << "FunctionCall: " << node->function_name << "()\n";
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

void Interpreter::Visit(FunctionCallExpression*) {}

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

Type SemanticAnalyzer::GetExpressionType(Expression* expr) {
  if (auto* num = dynamic_cast<NumberExpression*>(expr)) {
    return Type::Int();
  } else if (auto* var = dynamic_cast<VarExpression*>(expr)) {
    if (!current_class_name.empty()) {
      auto cls_it = global_sym_table.classes.find(current_class_name);
      if (cls_it != global_sym_table.classes.end()) {
        auto field_it = cls_it->second.fields.find(var->name);
        if (field_it != cls_it->second.fields.end()) {
          return field_it->second.type;
        }
      }
    }
    VariableInfo* var_info = current_scope->ResolveVariable(var->name);
    if (!var_info) {
      throw std::runtime_error("Undeclared variable '" + var->name + "'");
    }
    return var_info->type;
  } else if (auto* field = dynamic_cast<FieldAccessExpression*>(expr)) {
    const ClassInfo* cls = ResolveClassOfVar(field->object_name);
    auto it = cls->fields.find(field->field_name);
    if (it == cls->fields.end()) {
      throw std::runtime_error("No field '" + field->field_name + "' in class");
    }
    return it->second.type;
  } else if (auto* method_call = dynamic_cast<MethodCallExpression*>(expr)) {
    const ClassInfo* cls = ResolveClassOfVar(method_call->object_name);
    auto it = cls->methods.find(method_call->method_name);
    if (it == cls->methods.end()) {
      throw std::runtime_error("No method '" + method_call->method_name + "'");
    }
    return it->second.return_type;
  } else if (dynamic_cast<BinaryExpression*>(expr)) {
    return Type::Int();
  }
  return Type::Int();
}

void SemanticAnalyzer::Visit(NumberExpression*) {
  last_expr_type = Type::Int();
}

void SemanticAnalyzer::Visit(VarExpression* node) {
  if (!current_class_name.empty()) {
    auto cls_it = global_sym_table.classes.find(current_class_name);
    if (cls_it != global_sym_table.classes.end()) {
      auto field_it = cls_it->second.fields.find(node->name);
      if (field_it != cls_it->second.fields.end()) {
        last_expr_type = field_it->second.type;
        return;
      }
    }
  }
  VariableInfo* var = current_scope->ResolveVariable(node->name);
  if (!var) {
    throw std::runtime_error("Use of undeclared variable '" + node->name + "'");
  }
  last_expr_type = var->type;
}

void SemanticAnalyzer::Visit(BinaryExpression* node) {
  node->left->Accept(*this);
  node->right->Accept(*this);
  last_expr_type = Type::Int();
}

void SemanticAnalyzer::Visit(FieldAccessExpression* node) {
  const ClassInfo* cls = ResolveClassOfVar(node->object_name);
  auto it = cls->fields.find(node->field_name);
  if (it == cls->fields.end()) {
    throw std::runtime_error("No field '" + node->field_name + "' in class '" +
                             cls->name + "'");
  }
  last_expr_type = it->second.type;
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
  for (size_t i = 0; i < node->arguments.size(); ++i) {
    node->arguments[i]->Accept(*this);
    Type arg_type = last_expr_type;
    if (arg_type != it->second.arguments[i].type) {
      throw std::runtime_error("Argument type mismatch in method '" +
                             node->method_name + "'");
    }
  }
  last_expr_type = it->second.return_type;
}

void SemanticAnalyzer::Visit(FunctionCallExpression* node) {
  auto it = global_sym_table.global_methods.find(node->function_name);
  if (it == global_sym_table.global_methods.end()) {
    throw std::runtime_error("No global function '" + node->function_name + "'");
  }
  if (node->arguments.size() != it->second.arguments.size()) {
    throw std::runtime_error("Wrong number of arguments for function '" +
                             node->function_name + "'");
  }
  for (size_t i = 0; i < node->arguments.size(); ++i) {
    node->arguments[i]->Accept(*this);
    Type arg_type = last_expr_type;
    if (arg_type != it->second.arguments[i].type) {
      throw std::runtime_error("Argument type mismatch in function '" +
                             node->function_name + "'");
    }
  }
  last_expr_type = it->second.return_type;
}

void SemanticAnalyzer::Visit(DeclareStatement* node) {
  if (!current_scope->DeclareVariable(node->name, node->type)) {
    throw std::runtime_error("Variable '" + node->name +
                             "' already declared in this scope");
  }
}

void SemanticAnalyzer::Visit(AssignStatement* node) {
  if (!current_class_name.empty()) {
    auto cls_it = global_sym_table.classes.find(current_class_name);
    if (cls_it != global_sym_table.classes.end()) {
      auto field_it = cls_it->second.fields.find(node->name);
      if (field_it != cls_it->second.fields.end()) {
        node->expr->Accept(*this);
        if (last_expr_type != field_it->second.type) {
          throw std::runtime_error("Type mismatch in assignment to field '" +
                                 node->name + "'");
        }
        return;
      }
    }
  }
  VariableInfo* var = current_scope->ResolveVariable(node->name);
  if (!var) {
    throw std::runtime_error("Assign to undeclared variable '" + node->name +
                             "'");
  }
  node->expr->Accept(*this);
  if (last_expr_type != var->type) {
    throw std::runtime_error("Type mismatch in assignment to '" + node->name + "'");
  }
}

void SemanticAnalyzer::Visit(FieldAssignStatement* node) {
  const ClassInfo* cls = ResolveClassOfVar(node->object_name);
  auto it = cls->fields.find(node->field_name);
  if (it == cls->fields.end()) {
    throw std::runtime_error("No field '" + node->field_name + "' in class '" +
                             cls->name + "'");
  }
  node->expr->Accept(*this);
  if (last_expr_type != it->second.type) {
    throw std::runtime_error("Type mismatch in field assignment");
  }
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
    if (field->type.kind == TypeKind::CLASS) {
      if (global_sym_table.classes.find(field->type.class_name) ==
          global_sym_table.classes.end()) {
        throw std::runtime_error("Unknown class type '" + field->type.class_name +
                               "' for field '" + field->name + "'");
      }
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
  std::string prev_class = current_class_name;
  current_class_name = node->name;

  for (auto& field : node->fields) {
    current_scope->DeclareVariable(field->name, field->type);
  }

  for (auto& method : node->methods) {
    method->Accept(*this);
  }

  current_class_name = prev_class;
  current_scope = current_scope->parent;
}

void SemanticAnalyzer::Visit(MethodDeclarationStatement* node) {
  if (current_class_name.empty()) {
    if (global_sym_table.global_methods.find(node->data.name) !=
        global_sym_table.global_methods.end()) {
      throw std::runtime_error("Global method '" + node->data.name +
                             "' already declared");
    }
    global_sym_table.global_methods.emplace(node->data.name,
                      MethodInfo(node->data.name, node->data.return_type,
                                 node->data.arguments));
  }

  current_scope = current_scope->CreateChildScope();

  bool prev_in_method = in_method;
  Type prev_return_type = current_return_type;
  in_method = true;
  current_return_type = node->data.return_type;

  for (auto& arg : node->data.arguments) {
    if (arg.type.kind == TypeKind::CLASS) {
      if (global_sym_table.classes.find(arg.type.class_name) ==
          global_sym_table.classes.end()) {
        throw std::runtime_error("Unknown class type '" + arg.type.class_name +
                               "' for parameter '" + arg.name + "'");
      }
    }
    if (!current_scope->DeclareVariable(arg.name, arg.type)) {
      throw std::runtime_error("Duplicate argument name '" + arg.name + "'");
    }
  }

  if (node->data.return_type.kind == TypeKind::CLASS) {
    if (global_sym_table.classes.find(node->data.return_type.class_name) ==
        global_sym_table.classes.end()) {
      throw std::runtime_error("Unknown class type '" +
                             node->data.return_type.class_name +
                             "' for return type");
    }
  }

  node->data.body->Accept(*this);

  if (node->data.return_type.kind != TypeKind::VOID) {
    bool has_return = false;
    if (!node->data.body->statements.empty()) {
      auto* last_stmt = node->data.body->statements.back().get();
      has_return = dynamic_cast<ReturnStatement*>(last_stmt) != nullptr;
    }
    if (!has_return) {
      throw std::runtime_error("Non-void method '" + node->data.name +
                             "' must end with a return statement");
    }
  }

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
    if (last_expr_type != current_return_type) {
      throw std::runtime_error("Return type mismatch: expected " +
                             current_return_type.ToString() + ", got " +
                             last_expr_type.ToString());
    }
  }
}
