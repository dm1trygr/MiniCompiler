#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "types.hpp"
#include "utils.hpp"

struct GlobalSymbolTable {
  std::unordered_map<std::string, ClassInfo> classes;
  std::unordered_map<std::string, MethodInfo> global_methods;
};

class Scope {
 public:
  Scope(Scope* parent_def, GlobalSymbolTable& gst)
      : global_sym_table(gst), parent(parent_def) {}

  bool DeclareVariable(const std::string& name, const Type& type);
  VariableInfo* ResolveVariable(const std::string& name);

  GlobalSymbolTable& global_sym_table;
  Scope* parent;
  std::vector<std::unique_ptr<Scope>> children;
  std::unordered_map<std::string, VariableInfo> local_variables;
};
