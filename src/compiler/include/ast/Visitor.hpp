#pragma once

namespace raccoon::compiler::ast {

    class ExprStmt;
    class LiteralExpr;
    class VariableExpr;
    class FunctionExpr;
    class CallExpr;
    class UnaryExpression;
    class BinaryExpression;
    class BlockStmt;
    class VariableDecl;
    class VariableAssign;
    class FunctionDecl;
    class ReturnStmt;
    class DenStmt;
    class IfStmt;

    class Visitor {
    public:
        virtual ~Visitor() = default;
        virtual void visit(ExprStmt& node) = 0;
        virtual void visit(LiteralExpr& node) = 0;
        virtual void visit(VariableExpr& node) = 0;
        virtual void visit(FunctionExpr& node) = 0;
        virtual void visit(CallExpr& node) = 0;
        virtual void visit(UnaryExpression& node) = 0;
        virtual void visit(BinaryExpression& node) = 0;
        virtual void visit(BlockStmt& node) = 0;
        virtual void visit(VariableDecl& node) = 0;
        virtual void visit(VariableAssign& node) = 0;
        virtual void visit(FunctionDecl& node) = 0;
        virtual void visit(ReturnStmt& node) = 0;
        virtual void visit(DenStmt& node) = 0;
        virtual void visit(IfStmt& node) = 0;
    };
} // namespace raccoon::compiler::ast