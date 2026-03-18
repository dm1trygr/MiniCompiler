#include "parser.hpp"

#include <stdexcept>

std::unique_ptr<BlockStatement> Parser::parseProgram() {
  auto program = std::make_unique<BlockStatement>();
  while (peek().type != TokenType::END_OF_FILE) {
    program->statements.push_back(parseStatement());
  }
  return program;
}

std::unique_ptr<Statement> Parser::parseStatement() {
  if (peek().type == TokenType::DECLARE) {
    consume();
    std::string name = consume().value;
    expect(TokenType::COLON, "Expected ':' after variable name");
    expect(TokenType::INT_TYPE, "Expected 'int' type");
    expect(TokenType::SEMICOLON, "Expected ';'");
    return std::make_unique<DeclareStatement>(name);
  } else if (peek().type == TokenType::ID) {
    std::string name = consume().value;
    expect(TokenType::ASSIGN, "Expected '='");
    auto expr = parseExpression();
    expect(TokenType::SEMICOLON, "Expected ';'");
    return std::make_unique<AssignStatement>(name, std::move(expr));
  } else if (peek().type == TokenType::PRINT) {
    consume();
    expect(TokenType::LPAREN, "Expected '('");
    auto expr = parseExpression();
    expect(TokenType::RPAREN, "Expected ')'");
    expect(TokenType::SEMICOLON, "Expected ';'");
    return std::make_unique<PrintStatement>(std::move(expr));
  } else if (peek().type == TokenType::IF) {
    consume();
    expect(TokenType::LPAREN, "Expected '('");
    auto cond = parseExpression();
    expect(TokenType::RPAREN, "Expected ')'");
    auto thenBlock = parseBlock();

    std::unique_ptr<Statement> elseBlock = nullptr;
    if (peek().type == TokenType::ELSE) {
      consume();
      elseBlock = parseBlock();
    }
    return std::make_unique<IfStatement>(std::move(cond), std::move(thenBlock),
                                         std::move(elseBlock));
  }
  throw std::runtime_error("Unknown statement");
}

std::unique_ptr<BlockStatement> Parser::parseBlock() {
  expect(TokenType::LBRACE, "Expected '{'");
  auto block = std::make_unique<BlockStatement>();
  while (peek().type != TokenType::RBRACE &&
         peek().type != TokenType::END_OF_FILE) {
    block->statements.push_back(parseStatement());
  }
  expect(TokenType::RBRACE, "Expected '}'");
  return block;
}

std::unique_ptr<Expression> Parser::parseExpression() {
  std::unique_ptr<Expression> left;
  if (peek().type == TokenType::NUMBER) {
    left = std::make_unique<NumberExpression>(std::stoi(consume().value));
  } else if (peek().type == TokenType::ID) {
    left = std::make_unique<VarExpression>(consume().value);
  } else {
    throw std::runtime_error("Expected number / variable");
  }
  if (peek().type == TokenType::EQ) {
    std::string op = consume().value;
    std::unique_ptr<Expression> right;
    if (peek().type == TokenType::NUMBER) {
      right = std::make_unique<NumberExpression>(std::stoi(consume().value));
    } else if (peek().type == TokenType::ID) {
      right = std::make_unique<VarExpression>(consume().value);
    }
    return std::make_unique<BinaryExpression>(std::move(left), std::move(right),
                                              op);
  }
  return left;
}

Token Parser::peek() { return tokens[pos]; }

Token Parser::consume() { return tokens[pos++]; }

void Parser::expect(TokenType type, const std::string& err) {
  if (peek().type == type)
    consume();
  else
    throw std::runtime_error(err);
}
