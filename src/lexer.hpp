#pragma once

#include <string>
#include <vector>

enum class TokenType {
  DECLARE,
  INT_TYPE,
  VOID_TYPE,
  IF,
  ELSE,
  WHILE,
  PRINT,
  ID,
  NUMBER,
  CLASS,
  METHOD,
  RETURN,
  COMMA,
  ASSIGN,
  EQ,
  PLUS,
  MINUS,
  MULT,
  DIVIDE,
  COLON,
  SEMICOLON,
  LPAREN,
  RPAREN,
  LBRACE,
  RBRACE,
  END_OF_FILE
};

struct Token {
  TokenType type;
  std::string value;
};

class Lexer {
 public:
  explicit Lexer(const std::string& source) : src(source) {}

  std::vector<Token> Tokenize();

 private:
  std::string src;
  size_t pos = 0;
};
