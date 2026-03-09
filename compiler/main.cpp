#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <iostream>
#include <memory>
#include "Lexer.h"
#include "Parser.h"
#include "include/ASTPrinter.h"

int main() {
    auto Context = std::make_unique<llvm::LLVMContext>();
    auto Module = std::make_unique<llvm::Module>("Raccoon", *Context);
    llvm::IRBuilder<> Builder(*Context);

    Module->print(llvm::errs(), nullptr);

    std::string source = "let main be ()int = () { let x be int = 10; return x; }";
    raccoon::Lexer lexer(source);
    auto tokens = lexer.tokenize();

    for (const auto& token : tokens) {
        std::cout << "Token Type: " << static_cast<int>(token.type)
                  << " | Lexeme: [" << token.lexeme << "]" << std::endl;
    }

    raccoon::Parser parser(tokens);

    try {
        // 1. Get the AST root
        std::unique_ptr<raccoon::BlockStmt> root = parser.parse();

        // 2. Create the printer
        raccoon::ASTPrinter printer;

        // 3. Start the "walk"
        std::cout << "--- Raccoon AST ---" << std::endl;
        root->accept(printer);

    } catch (const raccoon::ParseError& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
    
    return 0;
}