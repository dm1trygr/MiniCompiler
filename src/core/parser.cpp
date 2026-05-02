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
    return ParseReturnStatement();
  }
  if (Peek().type == TokenType::DECLARE) {
    return ParseDeclareStatement();
  }
  if (Peek().type == TokenType::ID) {
    return ParseAssignStatement();
  }
  if (Peek().type == TokenType::PRINT) {
    return ParsePrintStatement();
  }
  if (Peek().type == TokenType::IF) {
    return ParseIfStatement();
  }
  if (Peek().type == TokenType::WHILE) {
    return ParseWhileStatement();
  }
  throw std::runtime_error("Unknown statement");
}

std::unique_ptr<Statement> Parser::ParseReturnStatement() {
  Consume();
  std::unique_ptr<Expression> expr = nullptr;
  if (Peek().type != TokenType::SEMICOLON) {
    expr = ParseExpression();
  }
  Expect(TokenType::SEMICOLON, "Expected ';' after return");
  return std::make_unique<ReturnStatement>(std::move(expr));
}

std::unique_ptr<Statement> Parser::ParseDeclareStatement() {
  Consume();
  std::string name = Consume().value;
  Expect(TokenType::COLON, "Expected ':' after variable name");
  Type type = ParseType();
  Expect(TokenType::SEMICOLON, "Expected ';'");
  return std::make_unique<DeclareStatement>(name, type);
}

std::unique_ptr<Statement> Parser::ParseAssignStatement() {
  std::string name = Consume().value;

  // Check for obj.field or obj.method()
  if (Peek().type == TokenType::DOT) {
    Consume();
    std::string member = Consume().value;

    if (Peek().type == TokenType::LPAREN) {
      // obj.method(args);
      Consume();
      auto args = ParseArgumentList();
      Expect(TokenType::RPAREN, "Expected ')'");
      Expect(TokenType::SEMICOLON, "Expected ';'");
      auto call = std::make_unique<MethodCallExpression>(
          name, member, std::move(args));
      return std::make_unique<ExpressionStatement>(std::move(call));
    }

    // obj.field = expr;
    Expect(TokenType::ASSIGN, "Expected '='");
    auto expr = ParseExpression();
    Expect(TokenType::SEMICOLON, "Expected ';'");
    return std::make_unique<FieldAssignStatement>(name, member,
                                                  std::move(expr));
  }

  // Regular assignment: var = expr;
  Expect(TokenType::ASSIGN, "Expected '='");
  auto expr = ParseExpression();
  Expect(TokenType::SEMICOLON, "Expected ';'");
  return std::make_unique<AssignStatement>(name, std::move(expr));
}

std::unique_ptr<Statement> Parser::ParsePrintStatement() {
  Consume();
  Expect(TokenType::LPAREN, "Expected '('");
  auto expr = ParseExpression();
  Expect(TokenType::RPAREN, "Expected ')'");
  Expect(TokenType::SEMICOLON, "Expected ';'");
  return std::make_unique<PrintStatement>(std::move(expr));
}

std::unique_ptr<Statement> Parser::ParseIfStatement() {
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

std::unique_ptr<Statement> Parser::ParseWhileStatement() {
  Consume();
  Expect(TokenType::LPAREN, "Expected '('");
  auto cond = ParseExpression();
  Expect(TokenType::RPAREN, "Expected ')'");
  auto body = ParseBlock();
  return std::make_unique<WhileStatement>(std::move(cond), std::move(body));
}

std::unique_ptr<ClassDeclarationStatement> Parser::ParseClassDeclaration() {
  Expect(TokenType::CLASS, "Expected 'class'");
  std::string name = Consume().value;
  Expect(TokenType::LBRACE, "Expected '{'");

  auto cls = std::make_unique<ClassDeclarationStatement>(name);

  while (Peek().type != TokenType::RBRACE &&
         Peek().type != TokenType::END_OF_FILE) {
    if (Peek().type == TokenType::DECLARE) {
      cls->fields.push_back(ParseFieldDeclaration());
    } else if (Peek().type == TokenType::METHOD) {
      cls->methods.push_back(ParseMethodDeclaration());
    } else {
      throw std::runtime_error("Expected field or method in class body");
    }
  }

  Expect(TokenType::RBRACE, "Expected '}'");
  return cls;
}

std::unique_ptr<DeclareStatement> Parser::ParseFieldDeclaration() {
  Consume();
  std::string field_name = Consume().value;
  Expect(TokenType::COLON, "Expected ':' after field name");
  Type type = ParseType();
  Expect(TokenType::SEMICOLON, "Expected ';'");
  return std::make_unique<DeclareStatement>(field_name, type);
}

std::unique_ptr<MethodDeclarationStatement> Parser::ParseMethodDeclaration() {
  Expect(TokenType::METHOD, "Expected 'method'");
  std::string name = Consume().value;
  Expect(TokenType::LPAREN, "Expected '('");

  auto args = ParseMethodArguments();

  Expect(TokenType::RPAREN, "Expected ')'");
  Expect(TokenType::COLON, "Expected ':' before return type");
  Type return_type = ParseType();

  auto body = ParseBlock();

  return std::make_unique<MethodDeclarationStatement>(
      name, return_type, std::move(args), std::move(body));
}

std::vector<VariableInfo> Parser::ParseMethodArguments() {
  std::vector<VariableInfo> args;

  if (Peek().type != TokenType::RPAREN) {
    std::string arg_name = Consume().value;
    Expect(TokenType::COLON, "Expected ':' after argument name");
    Type arg_type = ParseType();
    args.push_back({arg_name, arg_type});

    while (Peek().type == TokenType::COMMA) {
      Consume();
      arg_name = Consume().value;
      Expect(TokenType::COLON, "Expected ':' after argument name");
      arg_type = ParseType();
      args.push_back({arg_name, arg_type});
    }
  }

  return args;
}

Type Parser::ParseType() {
  if (Peek().type == TokenType::INT_TYPE) {
    Consume();
    return Type::Int();
  }
  if (Peek().type == TokenType::VOID_TYPE) {
    Consume();
    return Type::Void();
  }
  if (Peek().type == TokenType::ID) {
    std::string class_name = Consume().value;
    return Type::Class(class_name);
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
    BinaryOperator op = TokenTypeToOperator(Consume().type);
    auto right = ParseAdditive();
    return std::make_unique<BinaryExpression>(std::move(left), std::move(right),
                                              op);
  }
  return left;
}

std::unique_ptr<Expression> Parser::ParseAdditive() {
  auto left = ParseMultiplicative();
  while (Peek().type == TokenType::PLUS || Peek().type == TokenType::MINUS) {
    BinaryOperator op = TokenTypeToOperator(Consume().type);
    auto right = ParseMultiplicative();
    left = std::make_unique<BinaryExpression>(std::move(left), std::move(right),
                                              op);
  }
  return left;
}

std::unique_ptr<Expression> Parser::ParseMultiplicative() {
  auto left = ParsePrimary();
  while (Peek().type == TokenType::MULT || Peek().type == TokenType::DIVIDE) {
    BinaryOperator op = TokenTypeToOperator(Consume().type);
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
    std::string name = Consume().value;

    if (Peek().type == TokenType::DOT) {
      Consume();
      std::string member = Consume().value;

      if (Peek().type == TokenType::LPAREN) {
        Consume();
        auto args = ParseArgumentList();
        Expect(TokenType::RPAREN, "Expected ')'");
        return std::make_unique<MethodCallExpression>(name, member,
                                                      std::move(args));
      }

      return std::make_unique<FieldAccessExpression>(name, member);
    }

    return std::make_unique<VarExpression>(name);
  }

  if (Peek().type == TokenType::LPAREN) {
    Consume();
    auto expr = ParseExpression();
    Expect(TokenType::RPAREN, "Expected ')'");
    return expr;
  }

  throw std::runtime_error("Expected expression");
}

std::vector<std::unique_ptr<Expression>> Parser::ParseArgumentList() {
  std::vector<std::unique_ptr<Expression>> args;
  if (Peek().type == TokenType::RPAREN) return args;

  args.push_back(ParseExpression());
  while (Peek().type == TokenType::COMMA) {
    Consume();
    args.push_back(ParseExpression());
  }
  return args;
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
