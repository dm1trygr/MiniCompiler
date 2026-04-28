#pragma once

#include "lexer.hpp"
#include "statements.hpp"
#include "types.hpp"

class Parser {
 public:
  explicit Parser(const std::vector<Token>& t) : tokens(t) {}

  std::unique_ptr<BlockStatement> ParseProgram();

 private:
  std::unique_ptr<Statement> ParseStatement();
  std::unique_ptr<BlockStatement> ParseBlock();
  std::unique_ptr<ClassDeclarationStatement> ParseClassDeclaration();
  std::unique_ptr<MethodDeclarationStatement> ParseMethodDeclaration();
  Type ParseType();

  std::unique_ptr<Expression> ParseExpression();
  std::unique_ptr<Expression> ParseComparison();
  std::unique_ptr<Expression> ParseAdditive();
  std::unique_ptr<Expression> ParseMultiplicative();
  std::unique_ptr<Expression> ParsePrimary();

  Token Peek();
  Token Consume();
  void Expect(TokenType type, const std::string& err);

  std::vector<Token> tokens;
  size_t pos = 0;
};
