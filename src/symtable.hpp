#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "types.hpp"

struct VariableInfo {
  std::string name;
  Type type;
};

struct MethodInfo {
  std::string name;
  Type return_type;
  std::vector<VariableInfo> arguments;
};

struct ClassInfo {
  std::string name;
  std::unordered_map<std::string, VariableInfo> fields;
  std::unordered_map<std::string, MethodInfo> methods;
};

struct GlobalSymbolTable {
  std::unordered_map<std::string, ClassInfo> classes;
  std::unordered_map<std::string, MethodInfo> global_methods;
};

class Scope {
 public:
  Scope(Scope* parent_def, GlobalSymbolTable* gst)
      : parent(parent_def), global_sym_table(gst) {}

  bool DeclareVariable(const std::string& name, const Type& type);
  VariableInfo* ResolveVariable(const std::string& name);

 public:
  Scope* parent;
  std::vector<std::unique_ptr<Scope>> children;
  std::unordered_map<std::string, VariableInfo> local_variables;
  GlobalSymbolTable* global_sym_table;
};
