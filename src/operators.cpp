#include "operators.hpp"

#include <stdexcept>

std::string OperatorToString(BinaryOperator op) {
  switch (op) {
    case BinaryOperator::PLUS:
      return "+";
    case BinaryOperator::MINUS:
      return "-";
    case BinaryOperator::MULTIPLY:
      return "*";
    case BinaryOperator::DIVIDE:
      return "/";
    case BinaryOperator::EQUAL:
      return "==";
  }
  throw std::runtime_error("Unknown operator");
}

BinaryOperator TokenTypeToOperator(TokenType type) {
  switch (type) {
    case TokenType::PLUS:
      return BinaryOperator::PLUS;
    case TokenType::MINUS:
      return BinaryOperator::MINUS;
    case TokenType::MULT:
      return BinaryOperator::MULTIPLY;
    case TokenType::DIVIDE:
      return BinaryOperator::DIVIDE;
    case TokenType::EQ:
      return BinaryOperator::EQUAL;
    default:
      throw std::runtime_error("Not a binary operator token");
  }
}
