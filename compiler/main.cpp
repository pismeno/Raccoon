#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <iostream>
#include <memory>
#include "Lexer.cpp"

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
    
    return 0;
}