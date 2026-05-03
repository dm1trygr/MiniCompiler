#pragma once

#include "lexer.hpp"
#include "statements.hpp"
#include "types.hpp"
#include "utils.hpp"

class Parser {
 public:
  explicit Parser(const std::vector<Token>& t) : tokens(t) {}

  std::unique_ptr<BlockStatement> ParseProgram();

 private:
  std::unique_ptr<Statement> ParseStatement();
  std::unique_ptr<Statement> ParseReturnStatement();
  std::unique_ptr<Statement> ParseDeclareStatement();
  std::unique_ptr<Statement> ParseAssignStatement();
  std::unique_ptr<Statement> ParsePrintStatement();
  std::unique_ptr<Statement> ParseIfStatement();
  std::unique_ptr<Statement> ParseWhileStatement();

  std::unique_ptr<BlockStatement> ParseBlock();
  std::unique_ptr<ClassDeclarationStatement> ParseClassDeclaration();
  std::unique_ptr<DeclareStatement> ParseFieldDeclaration();
  std::unique_ptr<MethodDeclarationStatement> ParseMethodDeclaration();
  std::vector<VariableInfo> ParseMethodArguments();
  Type ParseType();

  std::unique_ptr<Expression> ParseExpression();
  std::unique_ptr<Expression> ParseComparison();
  std::unique_ptr<Expression> ParseAdditive();
  std::unique_ptr<Expression> ParseMultiplicative();
  std::unique_ptr<Expression> ParsePrimary();
  std::unique_ptr<Expression> ParseIdentifierExpression();

  std::vector<std::unique_ptr<Expression>> ParseArgumentList();

  std::unique_ptr<Statement> ParseMemberAccess(const std::string& object_name);
  std::unique_ptr<Statement> ParseMethodCallStatement(
      const std::string& object_name, const std::string& method_name);
  std::unique_ptr<Statement> ParseFieldAssignment(
      const std::string& object_name, const std::string& field_name);

  Token Peek();
  Token Consume();
  void Expect(TokenType type, const std::string& err);

 private:
  std::vector<Token> tokens;
  size_t pos = 0;
};
