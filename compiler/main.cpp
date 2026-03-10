#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <iostream>
#include <memory>
#include "include/Lexer.h"
#include "include/Parser.h"
#include "include/ASTPrinter.h"

int main() {
    auto Context = std::make_unique<llvm::LLVMContext>();
    auto Module = std::make_unique<llvm::Module>("Raccoon", *Context);
    llvm::IRBuilder<> Builder(*Context);

    Module->print(llvm::errs(), nullptr);

    std::string source = "let main be ()int = () { let x be int = 10; };";
    raccoon::Lexer lexer(source);
    auto tokens = lexer.tokenize();

    for (const auto& token : tokens) {
        std::cout << "Token Type: " << static_cast<int>(token.type)
                  << " | Lexeme: [" << token.lexeme << "]" << std::endl;
    }

    raccoon::Parser parser(tokens);

    try {
        std::unique_ptr<raccoon::BlockStmt> root = parser.parse();

        raccoon::ASTPrinter printer;

        std::cout << "--- Raccoon AST ---" << std::endl;
        root->accept(printer);

    } catch (const raccoon::ParseError& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}