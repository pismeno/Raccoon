#include "../../include/ast/AST.hpp"
#include "../../include/ast/VarTable.hpp"
#include "../../include/ast/SemanticAnalyzer.hpp"
#include "../../include/Parser.hpp"

namespace raccoon::compiler::ast {

    void SemanticAnalyzer::visit(ExprStmt &node) {
        if (currentExpectedFunctionType == nullptr) throw ParseError("Expression statements are not allowed outside of function.");
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
            throw ParseError("Signature mismatch: Function expression parameter count does not match the declaration.");
        }

        varTable.enterScope();

        for (size_t i = 0; i < node.paramNames.size(); ++i) {
            varTable.define(node.paramNames[i], signature->params[i], false);
        }

        bool previousHasReturnStmt = this->hasReturnStmt;
        std::shared_ptr<FunctionType> previousExpected = this->currentExpectedFunctionType;

        if (node.body) {
            node.body->accept(*this);
        }
        if (!this->hasReturnStmt && signature->returnType != PrimitiveType::Void) {
            throw ParseError("Function expression must return a value.");
        }

        this->hasReturnStmt = previousHasReturnStmt;
        this->currentExpectedFunctionType = previousExpected;

        varTable.exitScope();

        this->lastType = signature;
    }

    void SemanticAnalyzer::visit(ClassExpr &node) {
        if (!currentClassValue) {
            throw ParseError("Class expression must be assigned to a variable.");
        }

        for (const auto& member : node.statements) {
            if (member) member->accept(*this);
        }

        this->lastType = PrimitiveType::Class;
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
        node.left->accept(*this);
        auto leftType = this->lastType;

        node.right->accept(*this);
        auto rightType = this->lastType;

        if (!leftType || !rightType) {
            throw ParseError("Internal Error: Expression operands missing types.");
        }

        if (!(*leftType == *rightType)) {
            throw ParseError("Type mismatch: Cannot operate on " +
                             typeToString(leftType.get()) + " and " + typeToString(rightType.get()));
        }

        switch (node.op) {
            case ast::Operation::ADD:
            case ast::Operation::SUB:
            case ast::Operation::MUL:
            case ast::Operation::DIV:
                if (leftType->getKind() != TypeKind::INT && leftType->getKind() != TypeKind::FLOAT) {
                    throw ParseError("Arithmetic operators require numeric types (int or float).");
                }
                this->lastType = leftType;
                break;

            case ast::Operation::LESSER:
            case ast::Operation::GREATER:
            case ast::Operation::LESSER_EQUAL:
            case ast::Operation::GREATER_EQUAL:
                if (leftType->getKind() != TypeKind::INT && leftType->getKind() != TypeKind::FLOAT) {
                    throw ParseError("Relational operators require numeric types (int or float).");
                }
                this->lastType = PrimitiveType::Bool;
                break;

            case ast::Operation::EQUAL:
            case ast::Operation::NOT_EQUAL:
                this->lastType = PrimitiveType::Bool;
                break;

            case ast::Operation::AND:
            case ast::Operation::OR:
                if (leftType->getKind() != TypeKind::BOOL) {
                    throw ParseError("Logical operators (and, or) require boolean types.");
                }
                this->lastType = PrimitiveType::Bool;
                break;

            default:
                throw ParseError("Internal Error: Unhandled binary operation in Semantic Analyzer.");
        }
    }

    void SemanticAnalyzer::visit(BlockStmt &node) {
        bool insideClassExpr = currentClassValue != nullptr;
        bool insideFunctionExpr = currentExpectedFunctionType != nullptr;

        if (insideClassExpr && !insideFunctionExpr) {
            throw ParseError("Blocks are not allowed inside class expressions, outside of function expressions.");
        }

        varTable.enterScope();

        for (const auto& stmt : node.statements) {
            if (stmt) stmt->accept(*this);
        }

        varTable.exitScope();
    }

    void SemanticAnalyzer::visit(VariableDecl &node) {
        std::shared_ptr<Type> declaredType = checkType(node.type);

        if (currentClassValue) {
            if (currentClassValue->members.count(node.name)) {
                throw ParseError("Member '" + node.name + "' already defined in class " + currentClassValue->name);
            }

            size_t fieldIndex = currentClassValue->members.size();
            currentClassValue->members[node.name] = { declaredType, fieldIndex };

            if (node.initializer) {
                node.initializer->accept(*this);
                if (!(*this->lastType == *declaredType)) {
                    throw ParseError("Type mismatch in field '" + node.name + "' initializer.");
                }
            }
        } else {
            if (!varTable.isInDen()) throw ParseError("Declarations only allowed inside a den.");
            if (varTable.lookup(node.name)) throw ParseError("Variable '" + node.name + "' was already defined.");

            if (node.initializer) {
                node.initializer->accept(*this);
                if (!(*this->lastType == *declaredType)) {
                    throw ParseError("Type mismatch: Cannot assign " + typeToString(lastType.get()) + " to " + node.type);
                }
            } else {
                throw ParseError("Variable '" + node.name + "' requires an initializer.");
            }

            varTable.define(node.name, declaredType, node.isMutable);
        }
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

        if (currentClassValue) {
            if (currentClassValue->members.count(node.name)) {
                throw ParseError("Method '" + node.name + "' already defined in class " + currentClassValue->name);
            }

            size_t methodIndex = currentClassValue->members.size();
            currentClassValue->members[node.name] = { signature, methodIndex };

            if (node.initializer) {
                auto previousExpected = this->currentExpectedFunctionType;
                this->currentExpectedFunctionType = signature;

                node.initializer->accept(*this);

                this->currentExpectedFunctionType = previousExpected;
            }
        } else {
            if (!varTable.isInDen()) throw ParseError("Declarations only allowed inside a den.");
            if (varTable.lookup(node.name)) throw ParseError("Function '" + node.name + "' was already defined.");

            varTable.define(node.name, signature, node.isMutable);

            if (node.initializer) {
                auto previousExpected = this->currentExpectedFunctionType;
                this->currentExpectedFunctionType = signature;
                node.initializer->accept(*this);
                this->currentExpectedFunctionType = previousExpected;
            }
        }
    }

    void SemanticAnalyzer::visit(ClassDecl &node) {
        if (node.declaredMutable) throw ParseError("Cannot declare a class as mutable.");

        if (varTable.lookup(node.name)) throw ParseError("Name '" + node.name + "' already taken.");

        std::vector<std::shared_ptr<Type>> emptyParams;
        std::shared_ptr<Type> returnType = std::make_shared<ObjectType>(node.name);
        auto factorySignature = FunctionType::make(returnType, emptyParams);
        varTable.define(node.name, factorySignature, false);

        classRegistry[node.name] = { node.name, {} };

        auto* previousClass = this->currentClassValue;
        this->currentClassValue = &classRegistry[node.name];

        if (node.initializer) {
            node.initializer->accept(*this);
        }

        this->currentClassValue = previousClass;
    }

    void SemanticAnalyzer::visit(ReturnStmt &node) {
        if (node.expr) node.expr->accept(*this);
        else this->lastType = PrimitiveType::Void;
        if (this->currentExpectedFunctionType == nullptr) throw ParseError("Cannot return from a non-function.");
        if (*this->lastType != *this->currentExpectedFunctionType->returnType) {
            throw ParseError("Type mismatch: Function return type does not match expression type.");
        }
        this->hasReturnStmt = true;
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
        varTable.enterDens(node.name);

        for (const auto& stmt : node.contents) {
            if (stmt) stmt->accept(*this);
        }

        varTable.exitDens(node.name);
    }

    void SemanticAnalyzer::visit(IfStmt &node) {
        node.condition->accept(*this);
        if (*this->lastType != *PrimitiveType::Bool) {
            throw ParseError("Condition in if statement must be a boolean.");
        }

        node.thenBranch->accept(*this);
        if (node.elseBranch) node.elseBranch->accept(*this);
    }

    void SemanticAnalyzer::visit(ObjectDecl &node) {
        if (!varTable.isInDen()) {
            throw ParseError("Object declarations only allowed inside a den.");
        }

        if (varTable.lookup(node.name)) {
            throw ParseError("Name '" + node.name + "' already taken.");
        }

        auto classIt = classRegistry.find(node.className);
        if (classIt == classRegistry.end()) {
            throw ParseError("Undefined class: " + node.className);
        }

        auto objType = std::make_shared<ObjectType>(node.className);
        varTable.define(node.name, objType, node.declaredMutable);
        this->lastType = objType;

        if (node.initializer) {
            node.initializer->accept(*this);
        }
    }

    void SemanticAnalyzer::visit(MemberExpr &node) {
        std::optional<VarInfo> objInfo = varTable.lookup(node.object);
        if (!objInfo) {
            throw ParseError("Undefined object: " + node.object);
        }

        if (objInfo->type->getKind() != TypeKind::OBJECT) {
            throw ParseError("Object '" + node.object + "' is not a class instance.");
        }

        auto objType = std::static_pointer_cast<ObjectType>(objInfo->type);
        auto classIt = classRegistry.find(objType->classVariableName);
        if (classIt == classRegistry.end()) {
            throw ParseError("Object refers to undefined class: " + objType->classVariableName);
        }

        auto memberIt = classIt->second.members.find(node.member);
        if (memberIt == classIt->second.members.end()) {
            throw ParseError("Member '" + node.member + "' not found in class '" + objType->classVariableName + "'.");
        }

        this->lastType = memberIt->second.type;
    }

    std::shared_ptr<Type> SemanticAnalyzer::checkType(const std::string& declaredTypeStr) {
        std::shared_ptr<Type> declaredType = stringToType(declaredTypeStr);
        if (declaredType == nullptr) throw ParseError("Unknown type: " + declaredTypeStr);
        return declaredType;
    }
} // namespace raccoon::compiler::ast
