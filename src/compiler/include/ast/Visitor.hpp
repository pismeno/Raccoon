#pragma once

namespace raccoon::compiler::ast {

    class LiteralExpr;
    class VariableExpr;
    class FunctionDecl;
    class UnaryExpression;
    class BinaryExpression;
    class BlockStmt;
    class VariableDecl;
    class VariableAssign;
    class FunctionDecl;
    class DenStmt;

    class Visitor {
    public:
        virtual ~Visitor() = default;
        virtual void visit(LiteralExpr& node) = 0;
        virtual void visit(VariableExpr& node) = 0;
        virtual void visit(FunctionExpr& node) = 0;
        virtual void visit(UnaryExpression& node) = 0;
        virtual void visit(BinaryExpression& node) = 0;
        virtual void visit(BlockStmt& node) = 0;
        virtual void visit(VariableDecl& node) = 0;
        virtual void visit(VariableAssign& node) = 0;
        virtual void visit(FunctionDecl& node) = 0;
        virtual void visit(DenStmt& node) = 0;
    };
}