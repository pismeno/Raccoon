#include "../../include/ASTSemanticAnalyzer.h"
#include "../../include/AST.h"
#include "../../include/Parser.h"

namespace raccoon {

    void ASTSemanticAnalyzer::visit(LiteralExpr &node) {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, int64_t>) this->lastType = Type::INT;
            else if constexpr (std::is_same_v<T, double>) this->lastType = Type::FLOAT;
            else if constexpr (std::is_same_v<T, bool>)   this->lastType = Type::BOOL;
        }, node.value);
    }

    void ASTSemanticAnalyzer::visit(BlockStmt &node) {
        for (const auto& stmt : node.statements) {
            if (stmt) stmt->accept(*this);
        }
    }

    void ASTSemanticAnalyzer::visit(VariableDecl &node) {
        Type declaredType = stringToType(node.type);
        if (declaredType == Type::UNKNOWN) throw ParseError("Unknown type: " + node.type);

        if (node.initializer) {
            node.initializer->accept(*this);

            if (this->lastType != declaredType) {
                throw ParseError("Type mismatch: Cannot assign " + typeToString(lastType) + " to " + node.type);
            }
        }
    }

    void ASTSemanticAnalyzer::visit(FunctionDecl &node) {

    }

    ASTSemanticAnalyzer::ASTSemanticAnalyzer() {}
} // raccoon