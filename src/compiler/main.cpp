#include <llvm/IR/Module.h>
#include <iostream>
#include <memory>
#include "include/Lexer.hpp"
#include "include/Parser.hpp"
#include "include/ast/Printer.hpp"
#include "include/ast/SemanticAnalyzer.hpp"
#include "include/ast/IRGenerator.hpp"
#include <fstream>
#include <llvm/Support/raw_os_ostream.h>
#include "include/CLI11.hpp"

int main() {
    using namespace raccoon::compiler;

    CLI::App raccoonCli{"Raccoon Compiler"};

    std::string source = "den Main {"
                         "let add be (int, int)int = (a, b) {return a + b;};"
                         "let xx be mut int = 12;"
                         "let main be ()int = () {"
                         "let x be int = -1109 + 12; "
                         "let y be mut int = x * -2; "
                         "y = 99; "
                         "print(x - y);"
                         "return add(x, y); "
                         "};"
                         "let secon be mut ()int = main;"
                         "};";
    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    for (const auto& token : tokens) {
        std::cout << "Token Type: " << static_cast<int>(token.type)
                  << " | Lexeme: [" << token.lexeme << "]" << std::endl;
    }

    Parser parser(tokens);

    try {
        std::unique_ptr<ast::BlockStmt> root = parser.parse();

        ast::SemanticAnalyzer semanticAnalyzer;
        ast::Printer printer;
        ast::IRGenerator irGenerator;

        root->accept(semanticAnalyzer);

        std::cout << "--- Raccoon AST ---" << std::endl;
        root->accept(printer);

        std::cout << "--- Raccoon IR ---" << std::endl;
        root->accept(irGenerator);
        irGenerator.module->print(llvm::outs(), nullptr);

        std::ofstream file("output.ll");
        if (!file.is_open()) {
            std::cerr << "Could not open file 'output.ll' for writing." << std::endl;
            return 1;
        }

        llvm::raw_os_ostream dest(file);

        irGenerator.module->print(dest, nullptr);
        dest.flush();
        file.close();

        std::cout << "Successfully generated output.ll!" << std::endl;

    } catch (const ParseError& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}