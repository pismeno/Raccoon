#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <iostream>
#include <memory>

int main() {
    auto Context = std::make_unique<llvm::LLVMContext>();
    auto Module = std::make_unique<llvm::Module>("Raccoon", *Context);
    llvm::IRBuilder<> Builder(*Context);

    std::cout << "Raccoon Compiler is awake!" << std::endl;
    Module->print(llvm::errs(), nullptr);
    
    return 0;
}