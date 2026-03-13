#pragma once

#include "AST.hpp"
#include "Visitor.hpp"
#include <string>

namespace raccoon::compiler::ast {

    class Printer : public Visitor {
    public:
        Printer() : indentLevel(0) {}

        void visit(LiteralExpr& node) override;
        void visit(VariableExpr& node) override;
        void visit(UnaryExpression& node) override;
        void visit(BinaryExpression& node) override;
        void visit(BlockStmt& node) override;
        void visit(VariableDecl& node) override;
        void visit(VariableAssign& node) override;
        void visit(FunctionDecl& node) override;
        void visit(DenStmt& node) override;

    private:
        int indentLevel;
        void printIndent();
        std::string formatLiteral(const LiteralValue& value);
    };

} // namespace raccoon