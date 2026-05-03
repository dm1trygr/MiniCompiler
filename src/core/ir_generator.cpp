#include "ir_generator.hpp"

#include <llvm/IR/Verifier.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>

#include <stdexcept>

#include "operators.hpp"

IrGenerator::IrGenerator(Scope* root_scope, GlobalSymbolTable& global_sym_table)
    : module(std::make_unique<llvm::Module>("main_module", context)),
      builder(context),
      root_scope(root_scope),
      current_scope(root_scope),
      global_sym_table(global_sym_table) {}

llvm::Type* IrGenerator::GetLLVMType(const Type& type) {
  if (type.kind == TypeKind::INT) {
    return llvm::Type::getInt32Ty(context);
  } else if (type.kind == TypeKind::VOID) {
    return llvm::Type::getVoidTy(context);
  } else if (type.kind == TypeKind::CLASS) {
    if (class_types.contains(type.class_name)) {
      return class_types[type.class_name];
    }
    throw std::runtime_error("Unknown class type: " + type.class_name);
  }
  throw std::runtime_error("Unknown type");
}

void IrGenerator::EnterScope(Scope* scope) { current_scope = scope; }

void IrGenerator::ExitScope() {
  if (current_scope->parent) {
    current_scope = current_scope->parent;
  }
}

IrVarInfo* IrGenerator::LookupVariable(const std::string& name) {
  Scope* scope = current_scope;
  while (scope) {
    auto& scope_vars = scope_ir_vars[scope];
    auto found = scope_vars.find(name);
    if (found != scope_vars.end()) {
      return &found->second;
    }
    scope = scope->parent;
  }
  return nullptr;
}

llvm::AllocaInst* IrGenerator::CreateEntryAlloca(llvm::Function* fn,
                                                 const std::string& name,
                                                 llvm::Type* type) {
  llvm::IRBuilder<> tmp(&fn->getEntryBlock(), fn->getEntryBlock().begin());
  return tmp.CreateAlloca(type, nullptr, name);
}

llvm::Function* IrGenerator::GetOrDeclarePrintf() {
  llvm::Function* fn = module->getFunction("printf");
  if (fn) return fn;
  auto* type = llvm::FunctionType::get(
      llvm::Type::getInt32Ty(context),
      {llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(context))}, true);
  return llvm::Function::Create(type, llvm::Function::ExternalLinkage, "printf",
                                module.get());
}

int IrGenerator::GetFieldIndex(const std::string& class_name,
                               const std::string& field_name) {
  auto& fields = class_field_order[class_name];
  for (size_t i = 0; i < fields.size(); ++i) {
    if (fields[i] == field_name) return static_cast<int>(i);
  }
  throw std::runtime_error("IR: no field '" + field_name + "' in '" +
                           class_name + "'");
}

llvm::Value* IrGenerator::GetFieldPtr(const std::string& obj_name,
                                      const std::string& field_name) {
  IrVarInfo* info = LookupVariable(obj_name);
  if (!info) throw std::runtime_error("IR: undeclared '" + obj_name + "'");

  llvm::StructType* st = class_types[info->type.class_name];
  int idx = GetFieldIndex(info->type.class_name, field_name);

  return builder.CreateStructGEP(st, info->alloca_inst, idx,
                                 obj_name + "." + field_name + ".ptr");
}

void IrGenerator::GenerateClassesAndMethods(BlockStatement* program) {
  for (auto& stmt : program->statements) {
    if (dynamic_cast<ClassDeclarationStatement*>(stmt.get())) {
      stmt->Accept(*this);
    }
  }

  for (auto& stmt : program->statements) {
    if (dynamic_cast<MethodDeclarationStatement*>(stmt.get())) {
      stmt->Accept(*this);
    }
  }
}

void IrGenerator::GenerateMain(BlockStatement* program) {
  GenerateClassesAndMethods(program);

  auto* main_type =
      llvm::FunctionType::get(llvm::Type::getInt32Ty(context), false);
  auto* main_fn = llvm::Function::Create(
      main_type, llvm::Function::ExternalLinkage, "main", module.get());

  auto* entry = llvm::BasicBlock::Create(context, "entry", main_fn);
  builder.SetInsertPoint(entry);
  current_function = main_fn;

  program->Accept(*this);

  if (!builder.GetInsertBlock()->getTerminator()) {
    builder.CreateRet(llvm::ConstantInt::get(context, llvm::APInt(32, 0)));
  }
  current_function = nullptr;
  llvm::verifyModule(*module, &llvm::errs());
}

void IrGenerator::SaveToFile(const std::string& filename) const {
  std::error_code ec;
  llvm::raw_fd_ostream file(filename, ec, llvm::sys::fs::OF_Text);
  if (ec) throw std::runtime_error("Could not open file: " + ec.message());
  module->print(file, nullptr);
}

void IrGenerator::Visit(NumberExpression* node) {
  last_value =
      llvm::ConstantInt::get(context, llvm::APInt(32, node->value, true));
}

void IrGenerator::Visit(VarExpression* node) {
  if (current_this_ptr && !LookupVariable(node->name)) {
    llvm::StructType* st = class_types[current_class_name];
    int idx = GetFieldIndex(current_class_name, node->name);
    llvm::Value* field_ptr = builder.CreateStructGEP(
        st, current_this_ptr, idx, "this." + node->name + ".ptr");
    llvm::Type* field_type = st->getElementType(idx);
    last_value =
        builder.CreateLoad(field_type, field_ptr, "this." + node->name);
    return;
  }

  IrVarInfo* info = LookupVariable(node->name);
  if (!info) throw std::runtime_error("IR: undeclared '" + node->name + "'");
  llvm::Type* var_type = GetLLVMType(info->type);
  last_value = builder.CreateLoad(var_type, info->alloca_inst, node->name);
}

void IrGenerator::Visit(BinaryExpression* node) {
  node->left->Accept(*this);
  llvm::Value* l = last_value;
  node->right->Accept(*this);
  llvm::Value* r = last_value;

  switch (node->op) {
    case BinaryOperator::PLUS:
      last_value = builder.CreateAdd(l, r, "add");
      break;
    case BinaryOperator::MINUS:
      last_value = builder.CreateSub(l, r, "sub");
      break;
    case BinaryOperator::MULTIPLY:
      last_value = builder.CreateMul(l, r, "mul");
      break;
    case BinaryOperator::DIVIDE:
      last_value = builder.CreateSDiv(l, r, "div");
      break;
    case BinaryOperator::EQUAL:
      auto* cmp = builder.CreateICmpEQ(l, r, "eq");
      last_value =
          builder.CreateZExt(cmp, llvm::Type::getInt32Ty(context), "eqext");
      break;
  }
}

void IrGenerator::Visit(FieldAccessExpression* node) {
  IrVarInfo* info = LookupVariable(node->object_name);
  if (!info)
    throw std::runtime_error("IR: undeclared '" + node->object_name + "'");

  llvm::StructType* st = class_types[info->type.class_name];
  int idx = GetFieldIndex(info->type.class_name, node->field_name);

  llvm::Value* ptr = builder.CreateStructGEP(
      st, info->alloca_inst, idx,
      node->object_name + "." + node->field_name + ".ptr");
  llvm::Type* field_type = st->getElementType(idx);
  last_value = builder.CreateLoad(field_type, ptr,
                                  node->object_name + "." + node->field_name);
}

void IrGenerator::Visit(MethodCallExpression* node) {
  IrVarInfo* info = LookupVariable(node->object_name);
  if (!info)
    throw std::runtime_error("IR: undeclared '" + node->object_name + "'");

  std::string fn_name = info->type.class_name + "." + node->method_name;
  llvm::Function* fn = module->getFunction(fn_name);
  if (!fn) throw std::runtime_error("IR: no function '" + fn_name + "'");

  std::vector<llvm::Value*> args;
  args.push_back(info->alloca_inst);

  for (auto& arg : node->arguments) {
    arg->Accept(*this);
    args.push_back(last_value);
  }

  if (fn->getReturnType()->isVoidTy()) {
    builder.CreateCall(fn, args);
    last_value = nullptr;
  } else {
    last_value = builder.CreateCall(fn, args, "call");
  }
}

void IrGenerator::Visit(FunctionCallExpression* node) {
  llvm::Function* fn = module->getFunction(node->function_name);
  if (!fn)
    throw std::runtime_error("IR: no function '" + node->function_name + "'");

  std::vector<llvm::Value*> args;
  for (auto& arg : node->arguments) {
    arg->Accept(*this);
    args.push_back(last_value);
  }

  if (fn->getReturnType()->isVoidTy()) {
    builder.CreateCall(fn, args);
    last_value = nullptr;
  } else {
    last_value = builder.CreateCall(fn, args, "call");
  }
}

void IrGenerator::Visit(DeclareStatement* node) {
  if (node->type.kind == TypeKind::CLASS) {
    llvm::StructType* st = class_types[node->type.class_name];
    llvm::AllocaInst* alloca =
        CreateEntryAlloca(current_function, node->name, st);
    auto zero = llvm::ConstantAggregateZero::get(st);
    builder.CreateStore(zero, alloca);
    scope_ir_vars[current_scope][node->name] = {alloca, node->type};
  } else {
    llvm::AllocaInst* alloca = CreateEntryAlloca(
        current_function, node->name, llvm::Type::getInt32Ty(context));
    builder.CreateStore(llvm::ConstantInt::get(context, llvm::APInt(32, 0)),
                        alloca);
    scope_ir_vars[current_scope][node->name] = {alloca, node->type};
  }
}

void IrGenerator::Visit(AssignStatement* node) {
  if (current_this_ptr && !LookupVariable(node->name)) {
    llvm::StructType* st = class_types[current_class_name];
    int idx = GetFieldIndex(current_class_name, node->name);
    llvm::Value* field_ptr = builder.CreateStructGEP(
        st, current_this_ptr, idx, "this." + node->name + ".ptr");
    node->expr->Accept(*this);
    builder.CreateStore(last_value, field_ptr);
    return;
  }

  IrVarInfo* info = LookupVariable(node->name);
  if (!info) throw std::runtime_error("IR: undeclared '" + node->name + "'");
  node->expr->Accept(*this);
  builder.CreateStore(last_value, info->alloca_inst);
}

void IrGenerator::Visit(FieldAssignStatement* node) {
  llvm::Value* ptr = GetFieldPtr(node->object_name, node->field_name);
  node->expr->Accept(*this);
  builder.CreateStore(last_value, ptr);
}

void IrGenerator::Visit(ExpressionStatement* node) {
  node->expr->Accept(*this);
}

void IrGenerator::Visit(PrintStatement* node) {
  node->expr->Accept(*this);
  llvm::Function* printf_fn = GetOrDeclarePrintf();
  llvm::Value* fmt = builder.CreateGlobalStringPtr("%d\n", "fmt");
  builder.CreateCall(printf_fn, {fmt, last_value});
}

void IrGenerator::Visit(BlockStatement* node) {
  Scope* block_scope = current_scope;

  static int child_index = 0;
  if (!current_scope->children.empty() && current_scope != root_scope) {
    if (child_index < static_cast<int>(current_scope->children.size())) {
      block_scope = current_scope->children[child_index].get();
      child_index++;
    }
  }

  Scope* prev_scope = current_scope;
  EnterScope(block_scope);

  for (auto& stmt : node->statements) {
    if (!dynamic_cast<ClassDeclarationStatement*>(stmt.get()) &&
        !dynamic_cast<MethodDeclarationStatement*>(stmt.get())) {
      stmt->Accept(*this);
      if (builder.GetInsertBlock()->getTerminator()) break;
    }
  }

  current_scope = prev_scope;
  if (current_scope != root_scope) {
    child_index--;
  }
}

void IrGenerator::Visit(IfStatement* node) {
  node->condition->Accept(*this);
  auto* cond = builder.CreateICmpNE(
      last_value, llvm::ConstantInt::get(context, llvm::APInt(32, 0)), "ifc");

  auto* then_bb = llvm::BasicBlock::Create(context, "then", current_function);
  auto* else_bb = llvm::BasicBlock::Create(context, "else", current_function);
  auto* end_bb = llvm::BasicBlock::Create(context, "ifend", current_function);

  if (node->else_branch) {
    builder.CreateCondBr(cond, then_bb, else_bb);
  } else {
    builder.CreateCondBr(cond, then_bb, end_bb);
  }

  builder.SetInsertPoint(then_bb);
  node->then_branch->Accept(*this);
  if (!builder.GetInsertBlock()->getTerminator()) builder.CreateBr(end_bb);

  if (node->else_branch) {
    builder.SetInsertPoint(else_bb);
    node->else_branch->Accept(*this);
    if (!builder.GetInsertBlock()->getTerminator()) builder.CreateBr(end_bb);
  }

  builder.SetInsertPoint(end_bb);
}

void IrGenerator::Visit(WhileStatement* node) {
  auto* cond_bb = llvm::BasicBlock::Create(context, "wcond", current_function);
  auto* body_bb = llvm::BasicBlock::Create(context, "wbody", current_function);
  auto* end_bb = llvm::BasicBlock::Create(context, "wend", current_function);

  builder.CreateBr(cond_bb);
  builder.SetInsertPoint(cond_bb);
  node->condition->Accept(*this);
  auto* cond = builder.CreateICmpNE(
      last_value, llvm::ConstantInt::get(context, llvm::APInt(32, 0)), "wc");
  builder.CreateCondBr(cond, body_bb, end_bb);

  builder.SetInsertPoint(body_bb);
  node->body->Accept(*this);
  if (!builder.GetInsertBlock()->getTerminator()) builder.CreateBr(cond_bb);

  builder.SetInsertPoint(end_bb);
}

void IrGenerator::Visit(ClassDeclarationStatement* node) {
  std::vector<llvm::Type*> field_types;
  std::vector<std::string> field_names;
  for (auto& field : node->fields) {
    field_types.push_back(GetLLVMType(field->type));
    field_names.push_back(field->name);
  }

  auto* st = llvm::StructType::create(context, field_types, node->name);
  class_types[node->name] = st;
  class_field_order[node->name] = field_names;

  auto* prev_fn = current_function;
  auto* prev_bb = builder.GetInsertBlock();
  std::string prev_class = current_class_name;
  current_class_name = node->name;

  for (auto& method : node->methods) method->Accept(*this);

  current_class_name = prev_class;
  current_function = prev_fn;
  if (prev_bb) builder.SetInsertPoint(prev_bb);
}

void IrGenerator::Visit(MethodDeclarationStatement* node) {
  std::string fn_name = current_class_name.empty()
                            ? node->data.name
                            : current_class_name + "." + node->data.name;

  std::vector<llvm::Type*> arg_types;

  if (!current_class_name.empty()) {
    llvm::StructType* st = class_types[current_class_name];
    arg_types.push_back(llvm::PointerType::get(st, 0));
  }

  for (size_t i = 0; i < node->data.arguments.size(); ++i) {
    arg_types.push_back(GetLLVMType(node->data.arguments[i].type));
  }

  llvm::Type* ret = GetLLVMType(node->data.return_type);

  auto* ft = llvm::FunctionType::get(ret, arg_types, false);
  auto* fn = llvm::Function::Create(ft, llvm::Function::ExternalLinkage,
                                    fn_name, module.get());

  auto arg_it = fn->args().begin();

  if (!current_class_name.empty()) {
    arg_it->setName("this");
    ++arg_it;
  }

  for (size_t idx = 0; idx < node->data.arguments.size(); ++idx, ++arg_it) {
    arg_it->setName(node->data.arguments[idx].name);
  }

  auto* entry = llvm::BasicBlock::Create(context, "entry", fn);
  builder.SetInsertPoint(entry);
  current_function = fn;

  Scope* method_scope = nullptr;
  for (auto& child : current_scope->children) {
    if (!node->data.arguments.empty()) {
      if (child->local_variables.contains(node->data.arguments[0].name)) {
        method_scope = child.get();
        break;
      }
    } else {
      method_scope = child.get();
      break;
    }
  }

  if (!method_scope) {
    method_scope = current_scope;
  }

  Scope* prev_scope = current_scope;
  EnterScope(method_scope);

  arg_it = fn->args().begin();

  if (!current_class_name.empty()) {
    llvm::StructType* st = class_types[current_class_name];
    llvm::AllocaInst* this_alloca =
        CreateEntryAlloca(fn, "this.addr", llvm::PointerType::get(st, 0));
    builder.CreateStore(&(*arg_it), this_alloca);
    current_this_ptr =
        builder.CreateLoad(llvm::PointerType::get(st, 0), this_alloca, "this");
    ++arg_it;
  }

  for (size_t idx = 0; idx < node->data.arguments.size(); ++idx, ++arg_it) {
    auto* alloca =
        CreateEntryAlloca(fn, node->data.arguments[idx].name,
                          GetLLVMType(node->data.arguments[idx].type));
    builder.CreateStore(&(*arg_it), alloca);
    scope_ir_vars[current_scope][node->data.arguments[idx].name] = {
        alloca, node->data.arguments[idx].type};
  }

  node->data.body->Accept(*this);

  if (!builder.GetInsertBlock()->getTerminator()) {
    if (node->data.return_type.kind == TypeKind::VOID)
      builder.CreateRetVoid();
    else
      builder.CreateRet(llvm::ConstantInt::get(context, llvm::APInt(32, 0)));
  }

  current_scope = prev_scope;
  current_this_ptr = nullptr;
  llvm::verifyFunction(*fn, &llvm::errs());
}

void IrGenerator::Visit(ReturnStatement* node) {
  if (node->expr) {
    node->expr->Accept(*this);
    builder.CreateRet(last_value);
  } else {
    builder.CreateRetVoid();
  }
}
