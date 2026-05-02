#include <fstream>
#include <iostream>
#include <sstream>

#include "core/ir_generator.hpp"
#include "core/parser.hpp"
#include "core/semantic_analyzer.hpp"
#include "core/visitors.hpp"

std::string ReadFile(const std::string& path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open file: " + path);
  }
  std::stringstream ss;
  ss << file.rdbuf();
  return ss.str();
}

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Usage: ./compiler <source_file>\n";
    return 1;
  }

  try {
    std::string code = ReadFile(argv[1]);

    Lexer lexer(code);
    auto tokens = lexer.Tokenize();
    std::cout << "Tokens: " << tokens.size() << "\n";

    Parser parser(tokens);
    auto ast = parser.ParseProgram();
    std::cout << "AST built\n";

    SemanticAnalyzer analyzer;
    ast->Accept(analyzer);
    std::cout << "Semantic analysis passed\n";

    IrGenerator generator("main_module");
    generator.GenerateMain(ast.get());
    generator.SaveToFile("output.ll");
    std::cout << "IR saved to output.ll\n";

  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
