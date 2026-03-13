#include <llvm/IR/Module.h>
#include <iostream>
#include <memory>
#include "include/Lexer.hpp"
#include "include/Parser.hpp"
#include "include/ast/Printer.hpp"
#include "include/ast/SemanticAnalyzer.hpp"
#include "include/ast/IRGenerator.hpp"

int main() {

    using namespace raccoon::compiler;

    std::string source = "den Main {let main be ()int = () {let x be int = -1109 + 12; let y be mut int = x * -2; y = 99; };};";
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

    } catch (const ParseError& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}