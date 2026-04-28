#include "method_data.hpp"

#include "statements.hpp"

MethodData::MethodData(const std::string& n, const Type& ret,
                       std::vector<std::pair<std::string, Type>> args,
                       std::unique_ptr<BlockStatement> b)
    : name(n),
      arguments(std::move(args)),
      return_type(ret),
      body(std::move(b)) {}

MethodData::~MethodData() = default;
