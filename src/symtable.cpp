#include "symtable.hpp"

bool Scope::DeclareVariable(const std::string& name, const std::string& type) {
  if (local_variables.find(name) != local_variables.end()) {
    return false;
  }
  local_variables[name] = {name, type};
  return true;
}

VariableInfo* Scope::ResolveVariable(const std::string& name) {
  auto it = local_variables.find(name);
  if (it != local_variables.end()) {
    return &(it->second);
  }
  if (parent != nullptr) {
    return parent->ResolveVariable(name);
  }
  return nullptr;
}
