#include "../../include/ast/AST.hpp"
#include "../../include/ast/VarTable.hpp"
#include "../../include/ast/SemanticAnalyzer.hpp"
#include "../../include/Parser.hpp"

namespace raccoon::compiler::ast {

    void SemanticAnalyzer::visit(LiteralExpr &node) {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, int64_t>) this->lastType = PrimitiveType::Int;
            else if constexpr (std::is_same_v<T, double>) this->lastType = PrimitiveType::Float;
            else if constexpr (std::is_same_v<T, bool>)   this->lastType = PrimitiveType::Bool;
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
        std::shared_ptr<Type> declaredType = checkType(node.type);

        if (node.initializer) {
            node.initializer->accept(*this);

            if (!(*this->lastType == *declaredType)) {
                throw ParseError("Type mismatch: Cannot assign " + typeToString(lastType.get()) + " to " + node.type);
            }
        }

        varTable.define(node.name, declaredType, true);
    }

    void SemanticAnalyzer::visit(FunctionDecl &node) {

        std::shared_ptr<Type> returnType = checkType(node.returnType);
        std::vector<std::shared_ptr<Type>> paramTypes;
        paramTypes.reserve(node.paramTypes.size());
        for (const auto& paramType : node.paramTypes) {
                    paramTypes.push_back(checkType(paramType));
        }

        std::shared_ptr<FunctionType> signature = FunctionType::make(returnType, paramTypes);

        varTable.define(node.name, signature, true);

        if (node.body) {

            varTable.enterScope();

            for (size_t i = 0; i < node.paramNames.size(); ++i) {
                varTable.define(node.paramNames[i], paramTypes[i], true);
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

    std::shared_ptr<Type> SemanticAnalyzer::checkType(const std::string& declaredTypeStr) {
        std::shared_ptr<Type> declaredType = stringToType(declaredTypeStr);
        if (declaredType == nullptr) throw ParseError("Unknown type: " + declaredTypeStr);
        return declaredType;
    }
} // raccoon