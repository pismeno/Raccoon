#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace raccoon::compiler::ast {

    using LiteralValue = std::variant<int64_t, double, bool>;

    class Visitor;

    class ASTNode {
    public:
        virtual ~ASTNode() = default;

        virtual void accept(Visitor& visitor) = 0;
    };

    class Expr: public ASTNode {
    public:
        virtual ~Expr() = default;
    };

    class Stmt : public ASTNode {
    public:
        virtual ~Stmt() = default;
    };

    class LiteralExpr : public Expr {
    public:
        LiteralValue value;

        explicit LiteralExpr(LiteralValue val) : value(std::move(val)) {}

        void accept(Visitor& visitor) override;
    };

    class BlockStmt : public Stmt {
    public:
        std::vector<std::unique_ptr<Stmt>> statements;

        BlockStmt(std::vector<std::unique_ptr<Stmt>> statements): statements(std::move(statements)) {}

        void accept(Visitor& visitor) override;
    };

    class VariableDecl : public Stmt {
    public:
        std::string name;
        std::string type;
        std::unique_ptr<Expr> initializer;

        VariableDecl(std::string name, std::string type, std::unique_ptr<Expr> initializer):
        name(std::move(name)),
        type(std::move(type)),
        initializer(std::move(initializer)) {}

        void accept(Visitor& visitor) override;
    };

    class FunctionDecl : public Stmt {
    public:
        std::string name;
        std::vector<std::string> paramTypes;
        std::string returnType;
        std::vector<std::string> paramNames;
        std::unique_ptr<BlockStmt> body;

        FunctionDecl(std::string name, std::vector<std::string> paramTypes,
                     std::string returnType, std::vector<std::string> paramNames,
                     std::unique_ptr<BlockStmt> body)
                : name(std::move(name)), paramTypes(std::move(paramTypes)),
                  returnType(std::move(returnType)), paramNames(std::move(paramNames)),
                  body(std::move(body)) {}

        void accept(Visitor& visitor) override;
    };
} // raccoon