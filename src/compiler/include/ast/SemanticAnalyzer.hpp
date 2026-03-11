#pragma once

#include "Visitor.hpp"
#include "../Type.hpp"
#include <string>

namespace raccoon::compiler::ast {

    class SemanticAnalyzer: public Visitor {
    public:
        SemanticAnalyzer() = default;

        void visit(LiteralExpr& node) override;
        void visit(BlockStmt& node) override;
        void visit(VariableDecl& node) override;
        void visit(FunctionDecl& node) override;
    private:
        Type lastType = Type::UNKNOWN;

        static Type checkType(const std::string& declaredTypeStr);
    };

} // raccoon