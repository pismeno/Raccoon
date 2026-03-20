#pragma once

#include "Visitor.hpp"
#include "Type.hpp"
#include "VarTable.hpp"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include <string>

namespace raccoon::compiler::ast {

    class CompileError : public std::runtime_error {
    public:
        explicit CompileError(const std::string& message) : std::runtime_error(message) {}
    };

    class IRGenerator: public Visitor {
    public:
        IRGenerator();

        void visit(ExprStmt& node) override;
        void visit(LiteralExpr& node) override;
        void visit(VariableExpr& node) override;
        void visit(FunctionExpr& node) override;
        void visit(CallExpr& node) override;
        void visit(UnaryExpression& node) override;
        void visit(BinaryExpression& node) override;
        void visit(BlockStmt& node) override;
        void visit(VariableDecl& node) override;
        void visit(VariableAssign& node) override;
        void visit(FunctionDecl& node) override;
        void visit(ReturnStmt& node) override;
        void visit(DenStmt& node) override;

        std::unique_ptr<llvm::LLVMContext> context;
        std::unique_ptr<llvm::Module> module; // FIXME temporary in public
        std::unique_ptr<llvm::IRBuilder<>> builder;
    private:
        VarTable varTable;
        std::string currentDen = "";
        llvm::Value* lastValue = nullptr;

        std::shared_ptr<FunctionType> currentExpectedFunctionType = nullptr;
        std::string currentExpectedFuncName = "";
        bool hasReturnStmt = false;

        llvm::Type* getLLVMType(std::shared_ptr<Type> type);
    };

} // raccoon namespace::compiler::ast