#include <iostream>
#include <variant>
#include "../../include/ast/Printer.hpp"

namespace raccoon::compiler::ast {

    void Printer::printIndent() {
        for (int i = 0; i < indentLevel; ++i) std::cout << "  ";
    }

    std::string Printer::formatLiteral(const LiteralValue& value) {
        return std::visit([](auto&& arg) -> std::string {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, bool>) {
                return arg ? "true" : "false";
            } else if constexpr (std::is_same_v<T, std::string>) {
                return "\"" + arg + "\"";
            } else {
                return std::to_string(arg);
            }
        }, value);
    }

    void Printer::visit(ExprStmt& node) {
        printIndent();
        std::cout << "[ExprStmt]" << std::endl;
        if (node.expr) {
            indentLevel++;
            node.expr->accept(*this);
            indentLevel--;
        }
    }

    void Printer::visit(LiteralExpr& node) {
        printIndent();
        std::cout << "[Literal: " << formatLiteral(node.value) << "]" << std::endl;
    }

    void Printer::visit(VariableExpr& node) {
        printIndent();
        std::cout << "[Variable: " << node.name << "]" << std::endl;
    }

    void Printer::visit(FunctionExpr& node) {
        printIndent();
        std::cout << "[FunctionExpr: params=(";

        // Print out the parameter names separated by commas
        for (size_t i = 0; i < node.paramNames.size(); ++i) {
            std::cout << node.paramNames[i];
            if (i < node.paramNames.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << ")]" << std::endl;

        // Recursively print the body of the function
        if (node.body) {
            indentLevel++;
            node.body->accept(*this);
            indentLevel--;
        }
    }

    void Printer::visit(CallExpr& node) {
        printIndent();
        std::cout << "[CallExpr: func=" << node.func << "]" << std::endl;
        indentLevel++;
        for (const auto& arg : node.args) {
            if (arg) {
                arg->accept(*this);
            }
        }
        indentLevel--;
    }

    void Printer::visit(VariableDecl& node) {
        printIndent();
        std::cout << "[VariableDecl: name=" << node.name
                  << ", type=" << node.type << "]" << std::endl;

        if (node.initializer) {
            indentLevel++;
            node.initializer->accept(*this);
            indentLevel--;
        }
    }

    void Printer::visit(VariableAssign& node) {
        printIndent();
        std::cout << "VariableAssign(" << node.name << " =\n";
        indentLevel++;
        node.value->accept(*this);
        indentLevel--;
        printIndent();
        std::cout << ")\n";
    }

    void Printer::visit(MemberAssign &node) {
        printIndent();
        std::cout << "MemberAssign(\n";
        indentLevel++;
        printIndent();
        std::cout << "object:\n";
        indentLevel++;
        node.object->accept(*this);
        indentLevel--;
        printIndent();
        std::cout << "member: " << node.member << "\n";
        printIndent();
        std::cout << "value:\n";
        indentLevel++;
        node.value->accept(*this);
        indentLevel -= 2;
        printIndent();
        std::cout << ")\n";
    }

    void Printer::visit(UnaryExpression& node) {
        printIndent();
        std::cout << "[UnaryExpr: op='" << node.op << "']" << std::endl;

        indentLevel++;
        if (node.expr) {
            node.expr->accept(*this);
        }
        indentLevel--;
    }

    void Printer::visit(BinaryExpression& node) {
        printIndent();
        std::cout << "[BinaryExpr: op='" << node.op << "']" << std::endl;

        indentLevel++;
        if (node.left) {
            node.left->accept(*this);
        }
        if (node.right) {
            node.right->accept(*this);
        }
        indentLevel--;
    }

    void Printer::visit(FunctionDecl& node) {
        printIndent();
        std::cout << "[FunctionDecl: name=" << node.name
                  << ", isMutable=" << (node.isMutable ? "true" : "false")
                  << ", returnType=" << node.returnType
                  << ", paramTypes=(";

        // Print out the parameter types separated by commas
        for (size_t i = 0; i < node.paramTypes.size(); ++i) {
            std::cout << node.paramTypes[i];
            if (i < node.paramTypes.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << ")]" << std::endl;

        // Recursively print the initializer (which likely holds your FunctionExpr)
        if (node.initializer) {
            indentLevel++;
            node.initializer->accept(*this);
            indentLevel--;
        }
    }

    void Printer::visit(ClassDecl& node) {
        printIndent();
        std::cout << "[ClassDecl: name=" << node.name
                  << ", declaredMutable=" << (node.declaredMutable ? "true" : "false")
                  << "]" << std::endl;

        if (node.initializer) {
            indentLevel++;
            node.initializer->accept(*this);
            indentLevel--;
        }
    }

    void Printer::visit(ClassExpr &node) {
        printIndent();
        std::cout << "[ClassExpr]" << std::endl;
        indentLevel++;
        for (const auto& stmt : node.statements) {
            if (stmt) stmt->accept(*this);
        }
        indentLevel--;
    }

    void Printer::visit(BlockStmt& node) {
        printIndent();
        std::cout << "[BlockStmt]" << std::endl;
        indentLevel++;
        for (const auto& stmt : node.statements) {
            if (stmt) stmt->accept(*this);
        }
        indentLevel--;
    }

    void Printer::visit(ReturnStmt& node) {
        printIndent();
        std::cout << "[ReturnStmt]" << std::endl;
        if (node.expr) {
            indentLevel++;
            node.expr->accept(*this);
            indentLevel--;
        }
    }

    void Printer::visit(DenStmt& node) {
        printIndent();
        std::cout << "[DenStmt: name=" << node.name << "]" << std::endl;

        indentLevel++;
        for (const auto& stmt : node.contents) {
            if (stmt) {
                stmt->accept(*this);
            }
        }
        indentLevel--;
    }

    void Printer::visit(IfStmt& node) {
        printIndent();
        std::cout << "[IfStmt]" << std::endl;
        indentLevel++;
        node.condition->accept(*this);
        node.thenBranch->accept(*this);
        if (node.elseBranch) node.elseBranch->accept(*this);
        indentLevel--;
    }

    void Printer::visit(ObjectDecl& node) {
        printIndent();
        std::cout << "[ObjectDecl: name=" << node.name
                  << ", className=" << node.className
                  << ", declaredMutable=" << (node.declaredMutable ? "true" : "false")
                  << "]" << std::endl;

        if (node.initializer) {
            indentLevel++;
            node.initializer->accept(*this);
            indentLevel--;
        }
    }

    void Printer::visit(MemberExpr &node) {
        printIndent();
        std::cout << "[MemberExpr: member=" << node.member << "\n";
        indentLevel++;
        printIndent();
        std::cout << "object:\n";
        indentLevel++;
        node.object->accept(*this);
        indentLevel -= 2;
        printIndent();
        std::cout << "]\n";
    }

} // namespace raccoon::compiler::ast