#pragma once

#include "ASTVisitor.h"
#include "Type.h"
#include <string>

namespace raccoon {

    class ASTSemanticAnalyzer: public ASTVisitor {
    public:
        ASTSemanticAnalyzer();

        void visit(LiteralExpr& node) override;
        void visit(BlockStmt& node) override;
        void visit(VariableDecl& node) override;
        void visit(FunctionDecl& node) override;
    private:
        Type lastType = Type::UNKNOWN;
    };

} // raccoon