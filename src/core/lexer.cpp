#include "lexer.hpp"

#include <stdexcept>

void Lexer::SkipSpaces() {
  while (pos < src.length() && std::isspace(src[pos])) {
    pos++;
  }
}

Token Lexer::ReadIdentifierOrKeyword() {
  std::string word;
  while (pos < src.length() && (std::isalnum(src[pos]) || src[pos] == '_')) {
    word += src[pos++];
  }

  if (word == "declare")
    return {TokenType::DECLARE, word};
  else if (word == "int")
    return {TokenType::INT_TYPE, word};
  else if (word == "void")
    return {TokenType::VOID_TYPE, word};
  else if (word == "if")
    return {TokenType::IF, word};
  else if (word == "else")
    return {TokenType::ELSE, word};
  else if (word == "while")
    return {TokenType::WHILE, word};
  else if (word == "print")
    return {TokenType::PRINT, word};
  else if (word == "class")
    return {TokenType::CLASS, word};
  else if (word == "method")
    return {TokenType::METHOD, word};
  else if (word == "return")
    return {TokenType::RETURN, word};
  else
    return {TokenType::ID, word};
}

Token Lexer::ReadNumber() {
  std::string num;
  while (pos < src.length() && std::isdigit(src[pos])) {
    num += src[pos++];
  }
  return {TokenType::NUMBER, num};
}

Token Lexer::ReadEqualsOperator() {
  if (pos + 1 < src.length() && src[pos + 1] == '=') {
    pos += 2;
    return {TokenType::EQ, "=="};
  } else {
    pos++;
    return {TokenType::ASSIGN, "="};
  }
}

Token Lexer::ReadPunctuationOrOperator() {
  char current = src[pos++];

  switch (current) {
    case '+':
      return {TokenType::PLUS, "+"};
    case '-':
      return {TokenType::MINUS, "-"};
    case '*':
      return {TokenType::MULT, "*"};
    case '/':
      return {TokenType::DIVIDE, "/"};
    case '.':
      return {TokenType::DOT, "."};
    case ',':
      return {TokenType::COMMA, ","};
    case ':':
      return {TokenType::COLON, ":"};
    case ';':
      return {TokenType::SEMICOLON, ";"};
    case '(':
      return {TokenType::LPAREN, "("};
    case ')':
      return {TokenType::RPAREN, ")"};
    case '{':
      return {TokenType::LBRACE, "{"};
    case '}':
      return {TokenType::RBRACE, "}"};
    default:
      throw std::runtime_error(std::string("Unknown symbol: ") + current);
  }
}

std::vector<Token> Lexer::Tokenize() {
  std::vector<Token> tokens;

  while (pos < src.length()) {
    SkipSpaces();

    if (pos >= src.length()) {
      break;
    }

    char current = src[pos];

    if (std::isalpha(current) || current == '_') {
      tokens.push_back(ReadIdentifierOrKeyword());
      continue;
    }

    if (std::isdigit(current)) {
      tokens.push_back(ReadNumber());
      continue;
    }

    if (current == '=') {
      tokens.push_back(ReadEqualsOperator());
      continue;
    }

    tokens.push_back(ReadPunctuationOrOperator());
  }

  tokens.push_back({TokenType::END_OF_FILE, ""});
  return tokens;
}
