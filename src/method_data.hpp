#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "types.hpp"

class BlockStatement;

struct MethodData {
  std::string name;
  std::vector<std::pair<std::string, Type>> arguments;
  Type return_type;
  std::unique_ptr<BlockStatement> body;

  MethodData(const std::string& n, const Type& ret,
             std::vector<std::pair<std::string, Type>> args,
             std::unique_ptr<BlockStatement> b);

  ~MethodData();
};
