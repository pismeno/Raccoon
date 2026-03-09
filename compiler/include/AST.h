#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace raccoon {

    using LiteralValue = std::variant<int64_t, double, bool>;

    class ASTVisitor;

    class ASTNode {
    public:
        virtual ~ASTNode() = default;

        virtual void accept(ASTVisitor& visitor) = 0;
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

        void accept(ASTVisitor& visitor) override;
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

        void accept(ASTVisitor& visitor) override;
    };

    class BlockStmt : public Stmt {
        public:
        std::vector<std::unique_ptr<Stmt>> statements;

        BlockStmt(std::vector<std::unique_ptr<Stmt>> statements): statements(std::move(statements)) {}

        void accept(ASTVisitor& visitor) override;
    };
} // raccoon