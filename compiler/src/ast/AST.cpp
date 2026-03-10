#include "../../include/AST.h"
#include "../../include/ASTVisitor.h"

namespace raccoon {
    void LiteralExpr::accept(ASTVisitor& visitor) {
        visitor.visit(*this);
    }

    void BlockStmt::accept(ASTVisitor& visitor) {
        visitor.visit(*this);
    }

    void VariableDecl::accept(ASTVisitor& visitor) {
        visitor.visit(*this);
    }

    void FunctionDecl::accept(ASTVisitor &visitor) {
        visitor.visit(*this);
    }
} // raccoon