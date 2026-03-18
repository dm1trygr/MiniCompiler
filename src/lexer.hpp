#pragma once

#include <string>
#include <vector>

enum class TokenType {
  DECLARE,
  INT_TYPE,
  IF,
  ELSE,
  PRINT,
  ID,  // variable identifier/name
  NUMBER,
  ASSIGN,     // =
  EQ,         // ==
  COLON,      // :
  SEMICOLON,  // ;
  LPAREN,     // (
  RPAREN,     // )
  LBRACE,     // {
  RBRACE,     // }
  END_OF_FILE
};

struct Token {
  TokenType type;
  std::string value;
};

class Lexer {
  std::string src;
  size_t pos = 0;

 public:
  Lexer(const std::string& source) : src(source) {}

  std::vector<Token> tokenize();
};
