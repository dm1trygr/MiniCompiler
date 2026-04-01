#include "lexer.hpp"

#include <stdexcept>

std::vector<Token> Lexer::Tokenize() {
  std::vector<Token> tokens;
  while (pos < src.length()) {
    char current = src[pos];

    if (std::isspace(current)) {
      pos++;
      continue;
    }

    if (std::isalpha(current) || current == '_') {
      std::string word;
      while (pos < src.length() &&
             (std::isalnum(src[pos]) || src[pos] == '_')) {
        word += src[pos++];
      }
      if (word == "declare")
        tokens.push_back({TokenType::DECLARE, word});
      else if (word == "int")
        tokens.push_back({TokenType::INT_TYPE, word});
      else if (word == "void")
        tokens.push_back({TokenType::VOID_TYPE, word});
      else if (word == "if")
        tokens.push_back({TokenType::IF, word});
      else if (word == "else")
        tokens.push_back({TokenType::ELSE, word});
      else if (word == "while")
        tokens.push_back({TokenType::WHILE, word});
      else if (word == "print")
        tokens.push_back({TokenType::PRINT, word});
      else if (word == "class")
        tokens.push_back({TokenType::CLASS, word});
      else if (word == "method")
        tokens.push_back({TokenType::METHOD, word});
      else if (word == "return")
        tokens.push_back({TokenType::RETURN, word});
      else
        tokens.push_back({TokenType::ID, word});
      continue;
    }

    if (std::isdigit(current)) {
      std::string num;
      while (pos < src.length() && std::isdigit(src[pos])) {
        num += src[pos++];
      }
      tokens.push_back({TokenType::NUMBER, num});
      continue;
    }

    if (current == '=') {
      if (pos + 1 < src.length() && src[pos + 1] == '=') {
        tokens.push_back({TokenType::EQ, "=="});
        pos += 2;
      } else {
        tokens.push_back({TokenType::ASSIGN, "="});
        pos++;
      }
      continue;
    }

    switch (current) {
      case '+':
        tokens.push_back({TokenType::PLUS, "+"});
        break;
      case '-':
        tokens.push_back({TokenType::MINUS, "-"});
        break;
      case '*':
        tokens.push_back({TokenType::MULT, "*"});
        break;
      case '/':
        tokens.push_back({TokenType::DIVIDE, "/"});
        break;
      case ',':
        tokens.push_back({TokenType::COMMA, ","});
        break;
      case ':':
        tokens.push_back({TokenType::COLON, ":"});
        break;
      case ';':
        tokens.push_back({TokenType::SEMICOLON, ";"});
        break;
      case '(':
        tokens.push_back({TokenType::LPAREN, "("});
        break;
      case ')':
        tokens.push_back({TokenType::RPAREN, ")"});
        break;
      case '{':
        tokens.push_back({TokenType::LBRACE, "{"});
        break;
      case '}':
        tokens.push_back({TokenType::RBRACE, "}"});
        break;
      default:
        throw std::runtime_error(std::string("Unknown symbol: ") + current);
    }
    pos++;
  }
  tokens.push_back({TokenType::END_OF_FILE, ""});
  return tokens;
}
