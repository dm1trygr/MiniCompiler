#include "utils.hpp"

#include "statements.hpp"

MethodInfo::MethodInfo(const std::string& n, const Type& ret,
                       std::vector<VariableInfo> args,
                       std::unique_ptr<BlockStatement> b)
    : name(n),
      return_type(ret),
      arguments(std::move(args)),
      body(std::move(b)) {}

MethodInfo::MethodInfo(const std::string& n, const Type& ret,
                       const std::vector<VariableInfo>& args)
    : name(n), return_type(ret), arguments(args), body(nullptr) {}

MethodInfo::~MethodInfo() = default;
