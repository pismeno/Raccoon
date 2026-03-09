#pragma once

namespace raccoon {

    class LiteralExpr;
    class VariableDecl;
    class BlockStmt;

    class ASTVisitor {
    public:
        virtual ~ASTVisitor() = default;
        virtual void visit(LiteralExpr& node) = 0;
        virtual void visit(VariableDecl& node) = 0;
        virtual void visit(BlockStmt& node) = 0;
    };
}