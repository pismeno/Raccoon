#pragma once

#include "Visitor.hpp"
#include "Type.hpp"
#include "VarTable.hpp"
#include <string>

namespace raccoon::compiler::ast {

    class SemanticAnalyzer: public Visitor {
    public:
        SemanticAnalyzer() = default;

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
        void visit(IfStmt& node) override;
    private:
        VarTable varTable;
        std::shared_ptr<Type> lastType = PrimitiveType::Unknown;
        std::shared_ptr<FunctionType> currentExpectedFunctionType = nullptr;
        bool hasReturnStmt = false;

        static std::shared_ptr<Type> checkType(const std::string& declaredTypeStr);
    };

} // namespace raccoon::compiler::ast