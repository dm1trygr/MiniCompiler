#include "ir_generator.hpp"

#include <llvm/IR/Verifier.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>

#include <stdexcept>

#include "operators.hpp"

IrGenerator::IrGenerator(const std::string& module_name)
    : module(std::make_unique<llvm::Module>(module_name, context)),
      builder(context) {}

llvm::Type* IrGenerator::GetLLVMType(const Type& type) {
  if (type.kind == TypeKind::INT) {
    return llvm::Type::getInt32Ty(context);
  } else if (type.kind == TypeKind::VOID) {
    return llvm::Type::getVoidTy(context);
  } else if (type.kind == TypeKind::CLASS) {
    auto it = class_types.find(type.class_name);
    if (it != class_types.end()) {
      return it->second;
    }
    throw std::runtime_error("Unknown class type: " + type.class_name);
  }
  throw std::runtime_error("Unknown type");
}

void IrGenerator::PushScope() { scopes.emplace_back(); }

void IrGenerator::PopScope() { scopes.pop_back(); }

IrVarInfo* IrGenerator::LookupVariable(const std::string& name) {
  for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
    auto found = it->find(name);
    if (found != it->end()) return &found->second;
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
  return llvm::Function::Create(type, llvm::Function::ExternalLinkage,
                                "printf", module.get());
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

void IrGenerator::GenerateMain(BlockStatement* program) {
  for (auto& stmt : program->statements) {
    if (dynamic_cast<ClassDeclarationStatement*>(stmt.get())) {
      stmt->Accept(*this);
    }
  }

  auto* main_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(context),
                                            false);
  auto* main_fn = llvm::Function::Create(
      main_type, llvm::Function::ExternalLinkage, "main", module.get());

  auto* entry = llvm::BasicBlock::Create(context, "entry", main_fn);
  builder.SetInsertPoint(entry);
  current_function = main_fn;

  PushScope();

  for (auto& stmt : program->statements) {
    if (!dynamic_cast<ClassDeclarationStatement*>(stmt.get())) {
      stmt->Accept(*this);
      if (builder.GetInsertBlock()->getTerminator()) break;
    }
  }

  if (!builder.GetInsertBlock()->getTerminator()) {
    builder.CreateRet(llvm::ConstantInt::get(context, llvm::APInt(32, 0)));
  }

  PopScope();
  current_function = nullptr;
  llvm::verifyModule(*module, &llvm::errs());
}

void IrGenerator::DumpIr() const { module->print(llvm::outs(), nullptr); }

void IrGenerator::SaveToFile(const std::string& filename) const {
  std::error_code ec;
  llvm::raw_fd_ostream file(filename, ec, llvm::sys::fs::OF_Text);
  if (ec) throw std::runtime_error("Could not open file: " + ec.message());
  module->print(file, nullptr);
}

void IrGenerator::Visit(NumberExpression* node) {
  last_value = llvm::ConstantInt::get(context, llvm::APInt(32, node->value, true));
}

void IrGenerator::Visit(VarExpression* node) {
  IrVarInfo* info = LookupVariable(node->name);
  if (!info) throw std::runtime_error("IR: undeclared '" + node->name + "'");
  last_value = builder.CreateLoad(llvm::Type::getInt32Ty(context),
                                  info->alloca_inst, node->name);
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
      last_value = builder.CreateZExt(cmp, llvm::Type::getInt32Ty(context), "eqext");
      break;
  }
}

void IrGenerator::Visit(FieldAccessExpression* node) {
  llvm::Value* ptr = GetFieldPtr(node->object_name, node->field_name);
  last_value = builder.CreateLoad(llvm::Type::getInt32Ty(context), ptr,
                                  node->object_name + "." + node->field_name);
}

void IrGenerator::Visit(MethodCallExpression* node) {
  IrVarInfo* info = LookupVariable(node->object_name);
  if (!info) throw std::runtime_error("IR: undeclared '" + node->object_name + "'");

  std::string fn_name = info->type.class_name + "." + node->method_name;
  llvm::Function* fn = module->getFunction(fn_name);
  if (!fn) throw std::runtime_error("IR: no function '" + fn_name + "'");

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
    llvm::AllocaInst* alloca = CreateEntryAlloca(current_function, node->name, st);
    auto zero = llvm::ConstantAggregateZero::get(st);
    builder.CreateStore(zero, alloca);
    scopes.back()[node->name] = {alloca, node->type};
  } else {
    llvm::AllocaInst* alloca =
        CreateEntryAlloca(current_function, node->name,
                          llvm::Type::getInt32Ty(context));
    builder.CreateStore(llvm::ConstantInt::get(context, llvm::APInt(32, 0)),
                        alloca);
    scopes.back()[node->name] = {alloca, node->type};
  }
}

void IrGenerator::Visit(AssignStatement* node) {
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
  PushScope();
  for (auto& stmt : node->statements) {
    stmt->Accept(*this);
    if (builder.GetInsertBlock()->getTerminator()) break;
  }
  PopScope();
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
  for (size_t i = 0; i < node->data.arguments.size(); ++i) {
    arg_types.push_back(GetLLVMType(node->data.arguments[i].type));
  }

  llvm::Type* ret = GetLLVMType(node->data.return_type);

  auto* ft = llvm::FunctionType::get(ret, arg_types, false);
  auto* fn = llvm::Function::Create(ft, llvm::Function::ExternalLinkage,
                                    fn_name, module.get());

  size_t idx = 0;
  for (auto& arg : fn->args()) arg.setName(node->data.arguments[idx++].name);

  auto* entry = llvm::BasicBlock::Create(context, "entry", fn);
  builder.SetInsertPoint(entry);
  current_function = fn;

  PushScope();

  idx = 0;
  for (auto& arg : fn->args()) {
    auto* alloca = CreateEntryAlloca(fn, node->data.arguments[idx].name,
                                     GetLLVMType(node->data.arguments[idx].type));
    builder.CreateStore(&arg, alloca);
    scopes.back()[node->data.arguments[idx].name] = {alloca, node->data.arguments[idx].type};
    idx++;
  }

  node->data.body->Accept(*this);

  if (!builder.GetInsertBlock()->getTerminator()) {
    if (node->data.return_type.kind == TypeKind::VOID) builder.CreateRetVoid();
    else builder.CreateRet(llvm::ConstantInt::get(context, llvm::APInt(32, 0)));
  }

  PopScope();
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
