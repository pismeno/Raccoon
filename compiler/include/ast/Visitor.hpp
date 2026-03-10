#pragma once

namespace raccoon::compiler::ast {

    class LiteralExpr;
    class BlockStmt;
    class VariableDecl;
    class FunctionDecl;

    class Visitor {
    public:
        virtual ~Visitor() = default;
        virtual void visit(LiteralExpr& node) = 0;
        virtual void visit(BlockStmt& node) = 0;
        virtual void visit(VariableDecl& node) = 0;
        virtual void visit(FunctionDecl& node) = 0;
    };
}