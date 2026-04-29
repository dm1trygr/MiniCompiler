#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "types.hpp"

class BlockStatement;

struct VariableInfo {
  std::string name;
  Type type;
};

struct MethodInfo {
  std::string name;
  Type return_type;
  std::vector<VariableInfo> arguments;
  std::unique_ptr<BlockStatement> body;

  MethodInfo(const std::string& n, const Type& ret,
             std::vector<VariableInfo> args, std::unique_ptr<BlockStatement> b);

  MethodInfo(const std::string& n, const Type& ret,
             const std::vector<VariableInfo>& args);

  MethodInfo(MethodInfo&&) = default;
  MethodInfo& operator=(MethodInfo&&) = default;

  MethodInfo(const MethodInfo&) = delete;
  MethodInfo& operator=(const MethodInfo&) = delete;

  ~MethodInfo();
};

struct ClassInfo {
  std::string name;
  std::unordered_map<std::string, VariableInfo> fields;
  std::unordered_map<std::string, MethodInfo> methods;

  ClassInfo() = default;
  ClassInfo(ClassInfo&&) = default;
  ClassInfo& operator=(ClassInfo&&) = default;

  ClassInfo(const ClassInfo&) = delete;
  ClassInfo& operator=(const ClassInfo&) = delete;
};
