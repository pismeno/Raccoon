#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <iostream>
#include <memory>
#include "include/Lexer.hpp"
#include "include/Parser.hpp"
#include "include/ast/Printer.hpp"
#include "include/ast/SemanticAnalyzer.hpp"

int main() {

    using namespace raccoon::compiler;

    auto Context = std::make_unique<llvm::LLVMContext>();
    auto Module = std::make_unique<llvm::Module>("Raccoon", *Context);
    llvm::IRBuilder<> Builder(*Context);

    Module->print(llvm::errs(), nullptr);

    std::string source = "let x be int = 111111;";
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

        root->accept(semanticAnalyzer);

        std::cout << "--- Raccoon AST ---" << std::endl;
        root->accept(printer);

    } catch (const ParseError& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}