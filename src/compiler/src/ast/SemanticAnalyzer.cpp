#include "../../include/ast/AST.hpp"
#include "../../include/ast/VarTable.hpp"
#include "../../include/ast/SemanticAnalyzer.hpp"
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
        varTable.enterScope();

        for (const auto& stmt : node.statements) {
            if (stmt) stmt->accept(*this);
        }

        varTable.exitScope();
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
        std::string signature = "(";
        for (size_t i = 0; i < node.paramTypes.size(); ++i) {
            signature += node.paramTypes[i];
            if (i < node.paramTypes.size() - 1) signature += ",";
        }
        signature += ")" + node.returnType;

        varTable.define(node.name, signature, true);

        if (node.body) {

            varTable.enterScope();

            for (size_t i = 0; i < node.paramNames.size(); ++i) {
                varTable.define(node.paramNames[i], node.paramTypes[i], true);
            }

            for (const auto& stmt : node.body->statements) {
                if (stmt) stmt->accept(*this);
            }

            varTable.exitScope();
        } else {
            throw ParseError("Expected body for function: " + node.name);
        }
    }

    void SemanticAnalyzer::visit(DenStmt &node) {
        varTable.enterScope(node.name);

        for (const auto& stmt : node.contents) {
            if (stmt) stmt->accept(*this);
        }

        varTable.exitScope();
    }

    Type SemanticAnalyzer::checkType(const std::string& declaredTypeStr) {
        Type declaredType = stringToType(declaredTypeStr);
        if (declaredType == Type::UNKNOWN) throw ParseError("Unknown type: " + declaredTypeStr);
        return declaredType;
    }
} // raccoon