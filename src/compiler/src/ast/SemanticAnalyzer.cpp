#include "../../include/ast/AST.hpp"
#include "../../include/ast/VarTable.hpp"
#include "../../include/ast/SemanticAnalyzer.hpp"
#include "../../include/Parser.hpp"

namespace raccoon::compiler::ast {

    void SemanticAnalyzer::visit(ExprStmt &node) {
        if (node.expr) node.expr->accept(*this);
    }

    void SemanticAnalyzer::visit(LiteralExpr &node) {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, int32_t>) this->lastType = PrimitiveType::Int;
            else if constexpr (std::is_same_v<T, double>) this->lastType = PrimitiveType::Float;
            else if constexpr (std::is_same_v<T, bool>)   this->lastType = PrimitiveType::Bool;
        }, node.value);
    }

    void SemanticAnalyzer::visit(VariableExpr &node) {
        std::optional<VarInfo> varInfo = varTable.lookup(node.name);
        if (!varInfo) throw ParseError("Undefined variable: " + node.name);
        this->lastType = varInfo->type;
    }

    void SemanticAnalyzer::visit(FunctionExpr &node) {
        if (!this->currentExpectedFunctionType) {
            throw ParseError("Cannot infer parameter types for function expression. Expected signature missing.");
        }

        std::shared_ptr<FunctionType> signature = this->currentExpectedFunctionType;

        if (node.paramNames.size() != signature->params.size()) {
            throw ParseError("Arity mismatch: Function expression parameter count does not match the declaration.");
        }

        varTable.enterScope();

        for (size_t i = 0; i < node.paramNames.size(); ++i) {
            varTable.define(node.paramNames[i], signature->params[i], false);
        }

        if (node.body) {
            node.body->accept(*this);
        }

        this->currentExpectedFunctionType = nullptr;

        varTable.exitScope();

        this->lastType = signature;
    }

    void SemanticAnalyzer::visit(UnaryExpression &node) {
        if (node.expr) node.expr->accept(*this);
        std::shared_ptr<Type> rightType = this->lastType;

        if (node.op == "-") {
            if (rightType->getKind() != TypeKind::INT && rightType->getKind() != TypeKind::FLOAT) {
                throw ParseError("Type mismatch: Cannot negate a non-numeric type.");
            }
        } else if (node.op == "!") {
            if (rightType->getKind() != TypeKind::BOOL) {
                throw ParseError("Type mismatch: Cannot use '!' on a non-boolean type.");
            }
        }

        this->lastType = rightType;
    }

    void SemanticAnalyzer::visit(BinaryExpression &node) {
        if (node.left) node.left->accept(*this);
        std::shared_ptr<Type> leftType = this->lastType;

        if (node.right) node.right->accept(*this);
        std::shared_ptr<Type> rightType = this->lastType;

        if (!leftType || !rightType) {
            throw ParseError("Invalid expression types.");
        }

        if (!(*leftType == *rightType)) {
            throw ParseError("Type mismatch: Cannot operate on " +
                typeToString(leftType.get()) + " and " + typeToString(rightType.get()));
        }

        this->lastType = leftType;
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

        varTable.define(node.name, declaredType, node.isMutable);
    }

    void SemanticAnalyzer::visit(VariableAssign &node) {
        std::optional<VarInfo> varInfo = varTable.lookup(node.name);
        if (!varInfo) throw ParseError("Cannot assign to undefined variable: " + node.name);
        if (currentExpectedFunctionType == nullptr) throw ParseError("Cannot assign to " + node.name + " outside functions.");
        if (!varInfo->isMutable) throw ParseError("Cannot assign to immutable: " + node.name);
        this->lastType = varInfo->type;
    }

    void SemanticAnalyzer::visit(FunctionDecl &node) {
        std::shared_ptr<Type> returnType = checkType(node.returnType);
        std::vector<std::shared_ptr<Type>> paramTypes;
        paramTypes.reserve(node.paramTypes.size());
        for (const auto& paramType : node.paramTypes) {
            paramTypes.push_back(checkType(paramType));
        }

        std::shared_ptr<FunctionType> signature = FunctionType::make(returnType, paramTypes);

        varTable.define(node.name, signature, node.isMutable);

        if (node.initializer) {
            this->currentExpectedFunctionType = signature;

            node.initializer->accept(*this);

            if (!(*this->lastType == *signature)) {
                throw ParseError("Type mismatch: Function body does not match declaration signature for " + node.name);
            }
        }
    }

    void SemanticAnalyzer::visit(ReturnStmt &node) {
        if (node.expr) node.expr->accept(*this);
        if (this->currentExpectedFunctionType == nullptr) throw ParseError("Cannot return from a non-function.");
        if (!(*this->lastType == *this->currentExpectedFunctionType->returnType)) {
            throw ParseError("Type mismatch: Function return type does not match expression type.");
        }
    }

    void SemanticAnalyzer::visit(CallExpr &node) {

        if (node.func == "print") {
            for (const auto& arg : node.args) {
                if (arg) arg->accept(*this);
            }
            return;
        }

        std::optional<VarInfo> varInfo = varTable.lookup(node.func);
        if (!varInfo) {
            throw ParseError("Undefined function: " + node.func);
        }

        if (varInfo->type->getKind() != TypeKind::FUNCTION) {
            throw ParseError("Cannot call non-function: " + node.func);
        }

        auto funcType = std::static_pointer_cast<FunctionType>(varInfo->type);

        if (node.args.size() != funcType->params.size()) {
            throw ParseError("Function '" + node.func + "' expects " +
                             std::to_string(funcType->params.size()) + " arguments, but " +
                             std::to_string(node.args.size()) + " were provided.");
        }

        for (size_t i = 0; i < node.args.size(); ++i) {
            node.args[i]->accept(*this);

            if (*(this->lastType) != *(funcType->params[i])) {
                throw ParseError("Type mismatch in call to '" + node.func +
                                 "': Argument " + std::to_string(i) +
                                 " does not match expected type.");
            }
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
} // namespace raccoon::compiler::ast