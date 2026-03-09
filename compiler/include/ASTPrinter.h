#pragma once

#include "AST.h"
#include "ASTVisitor.h"
#include <string>

namespace raccoon {

    class ASTPrinter : public ASTVisitor {
    public:
        ASTPrinter() : indentLevel(0) {}

        void visit(LiteralExpr& node) override;
        void visit(VariableDecl& node) override;
        void visit(BlockStmt& node) override;

    private:
        int indentLevel;
        void printIndent();
        std::string formatLiteral(const LiteralValue& value);
    };

} // namespace raccoon