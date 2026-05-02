#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "types.hpp"
#include "visitors.hpp"

struct IrVarInfo {
  llvm::AllocaInst* alloca_inst;
  Type type;
};

class IrGenerator : public Visitor {
 public:
  explicit IrGenerator(const std::string& module_name);

  void Visit(NumberExpression* node) override;
  void Visit(VarExpression* node) override;
  void Visit(BinaryExpression* node) override;
  void Visit(FieldAccessExpression* node) override;
  void Visit(MethodCallExpression* node) override;

  void Visit(DeclareStatement* node) override;
  void Visit(AssignStatement* node) override;
  void Visit(FieldAssignStatement* node) override;
  void Visit(ExpressionStatement* node) override;
  void Visit(PrintStatement* node) override;
  void Visit(BlockStatement* node) override;
  void Visit(IfStatement* node) override;
  void Visit(WhileStatement* node) override;

  void Visit(ClassDeclarationStatement* node) override;
  void Visit(MethodDeclarationStatement* node) override;
  void Visit(ReturnStatement* node) override;

  void GenerateMain(BlockStatement* program);
  void DumpIr() const;
  void SaveToFile(const std::string& filename) const;

 private:
  llvm::LLVMContext context;
  std::unique_ptr<llvm::Module> module;
  llvm::IRBuilder<> builder;

  llvm::Value* last_value = nullptr;
  llvm::Function* current_function = nullptr;
  std::string current_class_name;

  std::vector<std::unordered_map<std::string, IrVarInfo>> scopes;
  std::unordered_map<std::string, llvm::StructType*> class_types;
  std::unordered_map<std::string, std::vector<std::string>> class_field_order;

  void PushScope();
  void PopScope();
  IrVarInfo* LookupVariable(const std::string& name);
  llvm::AllocaInst* CreateEntryAlloca(llvm::Function* fn,
                                      const std::string& name,
                                      llvm::Type* type);
  llvm::Function* GetOrDeclarePrintf();
  int GetFieldIndex(const std::string& class_name,
                    const std::string& field_name);
  llvm::Value* GetFieldPtr(const std::string& obj_name,
                           const std::string& field_name);
  llvm::Type* GetLLVMType(const Type& type);
};
