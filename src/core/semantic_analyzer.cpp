#include "semantic_analyzer.hpp"

#include <stdexcept>

SemanticAnalyzer::SemanticAnalyzer() {
  root_scope = std::make_unique<Scope>(nullptr, global_sym_table);
  current_scope = root_scope.get();
}

const ClassInfo* SemanticAnalyzer::ResolveClassOfVar(
    const std::string& var_name) {
  VariableInfo* var = current_scope->ResolveVariable(var_name);
  if (!var) {
    throw std::runtime_error("Undeclared variable '" + var_name + "'");
  }
  if (!global_sym_table.classes.contains(var->type.class_name)) {
    throw std::runtime_error("Variable '" + var_name +
                             "' is not of class type");
  }
  return &global_sym_table.classes.at(var->type.class_name);
}

Type SemanticAnalyzer::GetExpressionType(Expression* expr) {
  if (auto* num = dynamic_cast<NumberExpression*>(expr)) {
    return Type::Int();
  } else if (auto* var = dynamic_cast<VarExpression*>(expr)) {
    if (!current_class_name.empty()) {
      if (global_sym_table.classes.contains(current_class_name)) {
        auto& cls = global_sym_table.classes.at(current_class_name);
        if (cls.fields.contains(var->name)) {
          return cls.fields.at(var->name).type;
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
    if (!cls->fields.contains(field->field_name)) {
      throw std::runtime_error("No field '" + field->field_name + "' in class");
    }
    return cls->fields.at(field->field_name).type;
  } else if (auto* method_call = dynamic_cast<MethodCallExpression*>(expr)) {
    const ClassInfo* cls = ResolveClassOfVar(method_call->object_name);
    if (!cls->methods.contains(method_call->method_name)) {
      throw std::runtime_error("No method '" + method_call->method_name + "'");
    }
    return cls->methods.at(method_call->method_name).return_type;
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
    if (global_sym_table.classes.contains(current_class_name)) {
      auto& cls = global_sym_table.classes.at(current_class_name);
      if (cls.fields.contains(node->name)) {
        last_expr_type = cls.fields.at(node->name).type;
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
  if (!cls->fields.contains(node->field_name)) {
    throw std::runtime_error("No field '" + node->field_name + "' in class '" +
                             cls->name + "'");
  }
  last_expr_type = cls->fields.at(node->field_name).type;
}

void SemanticAnalyzer::Visit(MethodCallExpression* node) {
  const ClassInfo* cls = ResolveClassOfVar(node->object_name);
  if (!cls->methods.contains(node->method_name)) {
    throw std::runtime_error("No method '" + node->method_name +
                             "' in class '" + cls->name + "'");
  }
  const auto& method = cls->methods.at(node->method_name);
  if (node->arguments.size() != method.arguments.size()) {
    throw std::runtime_error("Wrong number of arguments for method '" +
                             node->method_name + "'");
  }
  for (size_t i = 0; i < node->arguments.size(); ++i) {
    node->arguments[i]->Accept(*this);
    Type arg_type = last_expr_type;
    if (arg_type != method.arguments[i].type) {
      throw std::runtime_error("Argument type mismatch in method '" +
                               node->method_name + "'");
    }
  }
  last_expr_type = method.return_type;
}

void SemanticAnalyzer::Visit(FunctionCallExpression* node) {
  if (!global_sym_table.global_methods.contains(node->function_name)) {
    throw std::runtime_error("No global function '" + node->function_name +
                             "'");
  }
  const auto& func = global_sym_table.global_methods.at(node->function_name);
  if (node->arguments.size() != func.arguments.size()) {
    throw std::runtime_error("Wrong number of arguments for function '" +
                             node->function_name + "'");
  }
  for (size_t i = 0; i < node->arguments.size(); ++i) {
    node->arguments[i]->Accept(*this);
    Type arg_type = last_expr_type;
    if (arg_type != func.arguments[i].type) {
      throw std::runtime_error("Argument type mismatch in function '" +
                               node->function_name + "'");
    }
  }
  last_expr_type = func.return_type;
}

void SemanticAnalyzer::Visit(DeclareStatement* node) {
  if (!current_scope->DeclareVariable(node->name, node->type)) {
    throw std::runtime_error("Variable '" + node->name +
                             "' already declared in this scope");
  }
}

void SemanticAnalyzer::Visit(AssignStatement* node) {
  if (!current_class_name.empty()) {
    if (global_sym_table.classes.contains(current_class_name)) {
      auto& cls = global_sym_table.classes.at(current_class_name);
      if (cls.fields.contains(node->name)) {
        node->expr->Accept(*this);
        if (last_expr_type != cls.fields.at(node->name).type) {
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
    throw std::runtime_error("Type mismatch in assignment to '" + node->name +
                             "'");
  }
}

void SemanticAnalyzer::Visit(FieldAssignStatement* node) {
  const ClassInfo* cls = ResolveClassOfVar(node->object_name);
  if (!cls->fields.contains(node->field_name)) {
    throw std::runtime_error("No field '" + node->field_name + "' in class '" +
                             cls->name + "'");
  }
  node->expr->Accept(*this);
  if (last_expr_type != cls->fields.at(node->field_name).type) {
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
  if (global_sym_table.classes.contains(node->name)) {
    throw std::runtime_error("Class '" + node->name + "' already declared");
  }

  std::unordered_map<std::string, VariableInfo> fields;
  std::unordered_map<std::string, MethodInfo> methods;

  for (auto& field : node->fields) {
    if (fields.contains(field->name)) {
      throw std::runtime_error("Field '" + field->name +
                               "' already declared in class '" + node->name +
                               "'");
    }
    if (field->type.kind == TypeKind::CLASS) {
      if (!global_sym_table.classes.contains(field->type.class_name)) {
        throw std::runtime_error("Unknown class type '" +
                                 field->type.class_name + "' for field '" +
                                 field->name + "'");
      }
    }
    fields[field->name] = {field->name, field->type};
  }

  for (auto& method : node->methods) {
    if (methods.contains(method->data.name)) {
      throw std::runtime_error("Method '" + method->data.name +
                               "' already declared in class '" + node->name +
                               "'");
    }

    methods.emplace(method->data.name,
                    MethodInfo(method->data.name, method->data.return_type,
                               method->data.arguments));
  }

  global_sym_table.classes.emplace(
      node->name, ClassInfo(node->name, std::move(fields), std::move(methods)));

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
    if (global_sym_table.global_methods.contains(node->data.name)) {
      throw std::runtime_error("Global method '" + node->data.name +
                               "' already declared");
    }
    global_sym_table.global_methods.emplace(
        node->data.name, MethodInfo(node->data.name, node->data.return_type,
                                    node->data.arguments));
  }

  current_scope = current_scope->CreateChildScope();

  bool prev_in_method = in_method;
  Type prev_return_type = current_return_type;
  in_method = true;
  current_return_type = node->data.return_type;

  for (auto& arg : node->data.arguments) {
    if (arg.type.kind == TypeKind::CLASS) {
      if (!global_sym_table.classes.contains(arg.type.class_name)) {
        throw std::runtime_error("Unknown class type '" + arg.type.class_name +
                                 "' for parameter '" + arg.name + "'");
      }
    }
    if (!current_scope->DeclareVariable(arg.name, arg.type)) {
      throw std::runtime_error("Duplicate argument name '" + arg.name + "'");
    }
  }

  if (node->data.return_type.kind == TypeKind::CLASS) {
    if (!global_sym_table.classes.contains(node->data.return_type.class_name)) {
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
