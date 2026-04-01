#include "parser.hpp"

#include <stdexcept>

std::unique_ptr<BlockStatement> Parser::ParseProgram() {
  auto program = std::make_unique<BlockStatement>();
  while (Peek().type != TokenType::END_OF_FILE) {
    program->statements.push_back(ParseStatement());
  }
  return program;
}

std::unique_ptr<Statement> Parser::ParseStatement() {
  if (Peek().type == TokenType::CLASS) {
    return ParseClassDeclaration();
  }

  if (Peek().type == TokenType::RETURN) {
    Consume();
    std::unique_ptr<Expression> expr = nullptr;
    if (Peek().type != TokenType::SEMICOLON) {
      expr = ParseExpression();
    }
    Expect(TokenType::SEMICOLON, "Expected ';' after return");
    return std::make_unique<ReturnStatement>(std::move(expr));
  }

  if (Peek().type == TokenType::DECLARE) {
    Consume();
    std::string name = Consume().value;
    Expect(TokenType::COLON, "Expected ':' after variable name");
    std::string type = ParseType();
    Expect(TokenType::SEMICOLON, "Expected ';'");
    return std::make_unique<DeclareStatement>(name, type);
  }

  if (Peek().type == TokenType::ID) {
    std::string name = Consume().value;
    Expect(TokenType::ASSIGN, "Expected '='");
    auto expr = ParseExpression();
    Expect(TokenType::SEMICOLON, "Expected ';'");
    return std::make_unique<AssignStatement>(name, std::move(expr));
  }

  if (Peek().type == TokenType::PRINT) {
    Consume();
    Expect(TokenType::LPAREN, "Expected '('");
    auto expr = ParseExpression();
    Expect(TokenType::RPAREN, "Expected ')'");
    Expect(TokenType::SEMICOLON, "Expected ';'");
    return std::make_unique<PrintStatement>(std::move(expr));
  }

  if (Peek().type == TokenType::IF) {
    Consume();
    Expect(TokenType::LPAREN, "Expected '('");
    auto cond = ParseExpression();
    Expect(TokenType::RPAREN, "Expected ')'");
    auto then_block = ParseBlock();

    std::unique_ptr<Statement> else_block = nullptr;
    if (Peek().type == TokenType::ELSE) {
      Consume();
      else_block = ParseBlock();
    }
    return std::make_unique<IfStatement>(std::move(cond), std::move(then_block),
                                         std::move(else_block));
  }

  if (Peek().type == TokenType::WHILE) {
    Consume();
    Expect(TokenType::LPAREN, "Expected '('");
    auto cond = ParseExpression();
    Expect(TokenType::RPAREN, "Expected ')'");
    auto body = ParseBlock();
    return std::make_unique<WhileStatement>(std::move(cond), std::move(body));
  }

  throw std::runtime_error("Unknown statement");
}

std::unique_ptr<ClassDeclarationStatement> Parser::ParseClassDeclaration() {
  Expect(TokenType::CLASS, "Expected 'class'");
  std::string name = Consume().value;
  Expect(TokenType::LBRACE, "Expected '{'");

  auto cls = std::make_unique<ClassDeclarationStatement>(name);

  while (Peek().type != TokenType::RBRACE &&
         Peek().type != TokenType::END_OF_FILE) {
    if (Peek().type == TokenType::DECLARE) {
      Consume();
      std::string field_name = Consume().value;
      Expect(TokenType::COLON, "Expected ':' after field name");
      std::string type = ParseType();
      Expect(TokenType::SEMICOLON, "Expected ';'");
      cls->fields.push_back(
          std::make_unique<DeclareStatement>(field_name, type));
    } else if (Peek().type == TokenType::METHOD) {
      cls->methods.push_back(ParseMethodDeclaration());
    } else {
      throw std::runtime_error("Expected field or method in class body");
    }
  }

  Expect(TokenType::RBRACE, "Expected '}'");
  return cls;
}

std::unique_ptr<MethodDeclarationStatement> Parser::ParseMethodDeclaration() {
  Expect(TokenType::METHOD, "Expected 'method'");
  std::string name = Consume().value;
  Expect(TokenType::LPAREN, "Expected '('");

  std::vector<std::pair<std::string, std::string>> args;
  if (Peek().type != TokenType::RPAREN) {
    std::string arg_name = Consume().value;
    Expect(TokenType::COLON, "Expected ':' after argument name");
    std::string arg_type = ParseType();
    args.push_back({arg_name, arg_type});

    while (Peek().type == TokenType::COMMA) {
      Consume();
      arg_name = Consume().value;
      Expect(TokenType::COLON, "Expected ':' after argument name");
      arg_type = ParseType();
      args.push_back({arg_name, arg_type});
    }
  }
  Expect(TokenType::RPAREN, "Expected ')'");

  Expect(TokenType::COLON, "Expected ':' before return type");
  std::string return_type = ParseType();

  auto body = ParseBlock();

  return std::make_unique<MethodDeclarationStatement>(
      name, return_type, std::move(args), std::move(body));
}

std::string Parser::ParseType() {
  if (Peek().type == TokenType::INT_TYPE) {
    return Consume().value;
  }
  if (Peek().type == TokenType::VOID_TYPE) {
    return Consume().value;
  }
  if (Peek().type == TokenType::ID) {
    return Consume().value;
  }
  throw std::runtime_error("Expected type name");
}

std::unique_ptr<BlockStatement> Parser::ParseBlock() {
  Expect(TokenType::LBRACE, "Expected '{'");
  auto block = std::make_unique<BlockStatement>();
  while (Peek().type != TokenType::RBRACE &&
         Peek().type != TokenType::END_OF_FILE) {
    block->statements.push_back(ParseStatement());
  }
  Expect(TokenType::RBRACE, "Expected '}'");
  return block;
}

std::unique_ptr<Expression> Parser::ParseExpression() {
  return ParseComparison();
}

std::unique_ptr<Expression> Parser::ParseComparison() {
  auto left = ParseAdditive();
  if (Peek().type == TokenType::EQ) {
    std::string op = Consume().value;
    auto right = ParseAdditive();
    return std::make_unique<BinaryExpression>(std::move(left), std::move(right),
                                              op);
  }
  return left;
}

std::unique_ptr<Expression> Parser::ParseAdditive() {
  auto left = ParseMultiplicative();
  while (Peek().type == TokenType::PLUS || Peek().type == TokenType::MINUS) {
    std::string op = Consume().value;
    auto right = ParseMultiplicative();
    left = std::make_unique<BinaryExpression>(std::move(left), std::move(right),
                                              op);
  }
  return left;
}

std::unique_ptr<Expression> Parser::ParseMultiplicative() {
  auto left = ParsePrimary();
  while (Peek().type == TokenType::MULT || Peek().type == TokenType::DIVIDE) {
    std::string op = Consume().value;
    auto right = ParsePrimary();
    left = std::make_unique<BinaryExpression>(std::move(left), std::move(right),
                                              op);
  }
  return left;
}

std::unique_ptr<Expression> Parser::ParsePrimary() {
  if (Peek().type == TokenType::NUMBER) {
    return std::make_unique<NumberExpression>(std::stoi(Consume().value));
  }
  if (Peek().type == TokenType::ID) {
    return std::make_unique<VarExpression>(Consume().value);
  }
  if (Peek().type == TokenType::LPAREN) {
    Consume();
    auto expr = ParseExpression();
    Expect(TokenType::RPAREN, "Expected ')'");
    return expr;
  }
  throw std::runtime_error("Expected expression");
}

Token Parser::Peek() { return tokens[pos]; }

Token Parser::Consume() { return tokens[pos++]; }

void Parser::Expect(TokenType type, const std::string& err) {
  if (Peek().type == type) {
    Consume();
  } else {
    throw std::runtime_error(err);
  }
}
