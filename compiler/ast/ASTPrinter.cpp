#include <iostream>
#include <variant>
#include "../include/ASTPrinter.h"

namespace raccoon {

    void ASTPrinter::printIndent() {
        for (int i = 0; i < indentLevel; ++i) std::cout << "  ";
    }

    std::string ASTPrinter::formatLiteral(const LiteralValue& value) {
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

    void ASTPrinter::visit(LiteralExpr& node) {
        printIndent();
        std::cout << "[Literal: " << formatLiteral(node.value) << "]" << std::endl;
    }

    void ASTPrinter::visit(VariableDecl& node) {
        printIndent();
        std::cout << "[VariableDecl: name=" << node.name
                  << ", type=" << node.type << "]" << std::endl;

        if (node.initializer) {
            indentLevel++;
            node.initializer->accept(*this);
            indentLevel--;
        }
    }

    void ASTPrinter::visit(BlockStmt& node) {
        printIndent();
        std::cout << "[BlockStmt]" << std::endl;
        indentLevel++;
        for (const auto& stmt : node.statements) {
            if (stmt) stmt->accept(*this);
        }
        indentLevel--;
    }

} // namespace raccoon