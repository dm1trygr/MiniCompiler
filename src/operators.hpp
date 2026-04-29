#pragma once

#include <string>

#include "lexer.hpp"

enum class BinaryOperator {
  PLUS,      // +
  MINUS,     // -
  MULTIPLY,  // *
  DIVIDE,    // /
  EQUAL      // ==
};

std::string OperatorToString(BinaryOperator op);
BinaryOperator TokenTypeToOperator(TokenType type);
