#include <cctype>
#include <iostream>

#include "core/parser.hpp"
#include "core/visitors.hpp"

int main() {
  // Here is code demonstration
  std::string code = R"(
        class Example {
            declare value: int;
            declare result: int;

            method add(a: int): int {
                return a + value;
            }

            method multiply(a: int): int {
                return a * value;
            }
        }

        declare x: int;
        x = 0;

        declare y: int;
        y = 5;

        if (x + 5 == 0) {
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
    auto tokens = lexer.Tokenize();
    std::cout << "Tokens count: " << tokens.size() << "\n\n";

    std::cout << "2) Building Tree\n";
    Parser parser(tokens);
    auto ast = parser.ParseProgram();
    std::cout << "Ok!\n\n";

    std::cout << "3) Saving Tree to file (Using Visitor)\n";
    PrintVisitor printer("ast_tree.txt");
    ast->Accept(printer);
    std::cout << "Tree saved to ast_tree.txt\n\n";

    std::cout << "4) Interpretation\n";
    Interpreter interpreter;
    ast->Accept(interpreter);
    std::cout << "Finished!\n\n";

    std::cout << "5) Semantic Analysis\n";
    SemanticAnalyzer analyzer;
    ast->Accept(analyzer);
    std::cout << "Ok!\n";
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << '\n';
  }

  return 0;
}
