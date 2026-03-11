#include "../../include/ast/SemanticAnalyzer.hpp"
#include "../../include/ast/AST.hpp"
#include "../../include/Parser.hpp"

namespace raccoon::compiler::ast {

    void SemanticAnalyzer::visit(LiteralExpr &node) {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, int64_t>) this->lastType = Type::INT;
            else if constexpr (std::is_same_v<T, double>) this->lastType = Type::FLOAT;
            else if constexpr (std::is_same_v<T, bool>)   this->lastType = Type::BOOL;
        }, node.value);
    }

    void SemanticAnalyzer::visit(BlockStmt &node) {
        for (const auto& stmt : node.statements) {
            if (stmt) stmt->accept(*this);
        }
    }

    void SemanticAnalyzer::visit(VariableDecl &node) {
        Type declaredType = checkType(node.type);

        if (node.initializer) {
            node.initializer->accept(*this);

            if (this->lastType != declaredType) {
                throw ParseError("Type mismatch: Cannot assign " + typeToString(lastType) + " to " + node.type);
            }
        }
    }

    void SemanticAnalyzer::visit(FunctionDecl &node) {
        Type returnType = checkType(node.returnType);
        std::vector<Type> signatureTypes;
        signatureTypes.reserve(node.paramTypes.size());
        for (const auto& paramType : node.paramTypes) {
                    signatureTypes.push_back(checkType(paramType));
        }
    }

    void SemanticAnalyzer::visit(DenStmt &node) {
        for (const auto& stmt : node.contents) {
            if (stmt) stmt->accept(*this);
        }
    }

    Type SemanticAnalyzer::checkType(const std::string& declaredTypeStr) {
        Type declaredType = stringToType(declaredTypeStr);
        if (declaredType == Type::UNKNOWN) throw ParseError("Unknown type: " + declaredTypeStr);
        return declaredType;
    }
} // raccoon