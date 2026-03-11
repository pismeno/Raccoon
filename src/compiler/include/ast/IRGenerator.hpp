#pragma once

#include "Visitor.hpp"
#include "../Type.hpp"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include <string>

namespace raccoon::compiler::ast {

    class IRGenerator: public Visitor {
    public:
        IRGenerator();

        void visit(LiteralExpr& node) override;
        void visit(BlockStmt& node) override;
        void visit(VariableDecl& node) override;
        void visit(FunctionDecl& node) override;

        std::unique_ptr<llvm::Module> module; // FIXME temporary in public
        std::unique_ptr<llvm::IRBuilder<>> builder;
        std::unique_ptr<llvm::LLVMContext> context;
    private:

        llvm::Value* lastValue = nullptr;

        llvm::Type* getLLVMType(const std::string& declaredTypeString);
    };

} // raccoon