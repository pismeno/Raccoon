#include "../../include/ast/AST.hpp"
#include "../../include/ast/Visitor.hpp"

namespace raccoon::compiler::ast {
    void LiteralExpr::accept(Visitor& visitor) {
        visitor.visit(*this);
    }

    void BlockStmt::accept(Visitor& visitor) {
        visitor.visit(*this);
    }

    void VariableDecl::accept(Visitor& visitor) {
        visitor.visit(*this);
    }

    void FunctionDecl::accept(Visitor &visitor) {
        visitor.visit(*this);
    }

    void DenStmt::accept(Visitor& visitor) {
        visitor.visit(*this);
    }
} // raccoon