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

        explicit LiteralExpr(LiteralValue val) : value(val) {}

        void accept(Visitor& visitor) override;
    };

    class VariableExpr : public Expr {
        public:
        std::string name;

        VariableExpr(std::string name) : name(std::move(name)) {}

        void accept(Visitor& visitor) override;
    };

    class UnaryExpression : public Expr {
    public:
        std::string op;
        std::unique_ptr<Expr> expr;

        UnaryExpression(std::string op, std::unique_ptr<Expr> expr) :
        op(std::move(op)),
        expr(std::move(expr)) {}

        void accept(Visitor& visitor) override;
    };

    class BinaryExpression : public Expr {
    public:
        std::string op;
        std::unique_ptr<Expr> left;
        std::unique_ptr<Expr> right;

        BinaryExpression(std::string op, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right) :
        op(std::move(op)),
        left(std::move(left)),
        right(std::move(right)) {}

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
        bool isMutable;
        std::string name;
        std::string type;
        std::unique_ptr<Expr> initializer;

        VariableDecl(std::string name, bool isMutable, std::string type, std::unique_ptr<Expr> initializer):
        name(std::move(name)),
        isMutable(isMutable),
        type(std::move(type)),
        initializer(std::move(initializer)) {}

        void accept(Visitor& visitor) override;
    };

    class VariableAssign : public Stmt {
    public:
        std::string name;
        std::unique_ptr<Expr> value;

        VariableAssign(std::string name, std::unique_ptr<Expr> value):
        name(std::move(name)),
        value(std::move(value)) {}

        void accept(Visitor& visitor) override;
    };

    class FunctionDecl : public Stmt {
    public:
        bool isMutable;
        std::string name;
        std::vector<std::string> paramTypes;
        std::string returnType;
        std::vector<std::string> paramNames;
        std::unique_ptr<BlockStmt> body;

        FunctionDecl(std::string name, bool isMutable, std::vector<std::string> paramTypes,
                     std::string returnType, std::vector<std::string> paramNames,
                     std::unique_ptr<BlockStmt> body):
        name(std::move(name)),
        isMutable(isMutable),
        paramTypes(std::move(paramTypes)),
        returnType(std::move(returnType)),
        paramNames(std::move(paramNames)),
        body(std::move(body)) {}

        void accept(Visitor& visitor) override;
    };

    class DenStmt: public Stmt {
    public:
        std::string name;
        std::vector<std::unique_ptr<Stmt>> contents;

        DenStmt(std::string name, std::vector<std::unique_ptr<Stmt>> contents):
        name(std::move(name)),
        contents(std::move(contents)) {}

        void accept(Visitor& visitor) override;
    };
} // raccoon