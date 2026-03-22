#include <cctype>
#include <iostream>

#include "src/parser.hpp"
#include "src/visitors.hpp"

int main() {
  // Here is code demonstration
  std::string code = R"(
        declare x: int;
        x = 0;
        
        declare y: int;
        y = 5;

        if (x == 0) {
            print(100);
            x = 1;
        } else {
            print(404);
        }

        if (x == 0) {
            print(999);
        } else {
            print(y);
        }
    )";

  try {
    std::cout << "1) Lexer\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    std::cout << "Tokens count: " << tokens.size() << "\n\n";

    std::cout << "2) Building Tree\n";
    Parser parser(tokens);
    auto ast = parser.parseProgram();

    std::cout << "3) Saving Tree to file (Using Visitor)\n";
    PrintVisitor printer("ast_tree.txt");
    ast->accept(printer);
    std::cout << "Tree saved to ast_tree.txt\n\n";

    std::cout << "3) Interpretation\n";
    Context ctx;
    Interpreter interpreter;
    ast->accept(interpreter);

  } catch (const std::exception& e) {
    std::cerr << "Syntax error: " << e.what() << '\n';
  }

  return 0;
}