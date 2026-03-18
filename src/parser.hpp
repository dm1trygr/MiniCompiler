#pragma once

#include "lexer.hpp"
#include "statements.hpp"

class Parser {
 public:
  Parser(const std::vector<Token>& t) : tokens(t) {}
  std::unique_ptr<BlockStatement> parseProgram();
  std::unique_ptr<Statement> parseStatement();
  std::unique_ptr<BlockStatement> parseBlock();
  std::unique_ptr<Expression> parseExpression();

 private:
  std::vector<Token> tokens;
  size_t pos = 0;

 private:
  Token peek();
  Token consume();
  void expect(TokenType type, const std::string& err);
};
