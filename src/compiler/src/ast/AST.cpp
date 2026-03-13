#include "../../include/ast/AST.hpp"
#include "../../include/ast/Visitor.hpp"

namespace raccoon::compiler::ast {
    void LiteralExpr::accept(Visitor& visitor) {
        visitor.visit(*this);
    }

    void VariableExpr::accept(Visitor& visitor) {
        visitor.visit(*this);
    }

    void FunctionExpr::accept(Visitor& visitor) {
        visitor.visit(*this);
    }

    void UnaryExpression::accept(Visitor& visitor) {
        visitor.visit(*this);
    }

    void BinaryExpression::accept(Visitor& visitor) {
        visitor.visit(*this);
    }

    void BlockStmt::accept(Visitor& visitor) {
        visitor.visit(*this);
    }

    void VariableDecl::accept(Visitor& visitor) {
        visitor.visit(*this);
    }

    void VariableAssign::accept(Visitor& visitor) {
        visitor.visit(*this);
    }

    void FunctionDecl::accept(Visitor &visitor) {
        visitor.visit(*this);
    }

    void DenStmt::accept(Visitor& visitor) {
        visitor.visit(*this);
    }
} // raccoon