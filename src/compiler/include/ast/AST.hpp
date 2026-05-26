#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <variant>
#include <vector>
#include "Operation.h"

namespace raccoon::compiler::ast {

    using LiteralValue = std::variant<int32_t, double, bool>;

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

    class ExprStmt : public Stmt {
    public:
        std::unique_ptr<Expr> expr;

        ExprStmt(std::unique_ptr<Expr> expr) : expr(std::move(expr)) {}

        void accept(Visitor& visitor) override;
    };

    class BlockStmt : public Stmt {
    public:
        std::vector<std::unique_ptr<Stmt>> statements;

        BlockStmt(std::vector<std::unique_ptr<Stmt>> statements): statements(std::move(statements)) {}

        void accept(Visitor& visitor) override;
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

    class MemberExpr : public Expr {
      public:
        std::string object;
        std::string member;

        MemberExpr(std::string object, std::string member) : object(std::move(object)), member(std::move(member)) {}

        void accept(Visitor& visitor) override;
    };

    class FunctionExpr : public Expr {
    public:
        std::vector<std::string> paramNames;
        std::unique_ptr<BlockStmt> body;

        FunctionExpr(std::vector<std::string> paramNames, std::unique_ptr<BlockStmt> body) :
        paramNames(std::move(paramNames)),
        body(std::move(body)) {}

        void accept(Visitor& visitor) override;
    };

    class CallExpr : public Expr {
    public:
        std::string func;
        std::vector<std::unique_ptr<Expr>> args;

        CallExpr(std::string func, std::vector<std::unique_ptr<Expr>> args):
                func(std::move(func)),
                args(std::move(args)) {}

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
        Operation op;
        std::unique_ptr<Expr> left;
        std::unique_ptr<Expr> right;

        BinaryExpression(Operation op, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right) :
        op(op),
        left(std::move(left)),
        right(std::move(right)) {}

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

    class MemberAssign : public Stmt {
    public:
        std::string object;
        std::string member;
        std::unique_ptr<Expr> value;

        MemberAssign(std::string object, std::string member, std::unique_ptr<Expr> value):
        object(std::move(object)),
        member(std::move(member)),
        value(std::move(value)) {}

        void accept(Visitor& visitor) override;
    };

    class FunctionDecl : public Stmt {
    public:
        bool isMutable;
        std::string name;
        std::vector<std::string> paramTypes;
        std::string returnType;
        std::unique_ptr<Expr> initializer;

        FunctionDecl(std::string name, bool isMutable, std::vector<std::string> paramTypes,
                     std::string returnType, std::unique_ptr<Expr> initializer):
        name(std::move(name)),
        isMutable(isMutable),
        paramTypes(std::move(paramTypes)),
        returnType(std::move(returnType)),
        initializer(std::move(initializer)) {}

        void accept(Visitor& visitor) override;
    };

    class ClassDecl : public Stmt {
    public:
        bool declaredMutable;
        std::string name;
        std::unique_ptr<Expr> initializer;

        ClassDecl(std::string name, bool declaredMutable, std::unique_ptr<Expr> initializer):
        declaredMutable(declaredMutable),
        name(std::move(name)),
        initializer(std::move(initializer)) {}

        void accept(Visitor& visitor) override;
    };

    class ObjectDecl : public Stmt {
    public:
        bool declaredMutable;
        std::string name;
        std::string className;
        std::unique_ptr<Expr> initializer;

        ObjectDecl(std::string name, bool declaredMutable, std::string className, std::unique_ptr<Expr> initializer):
        declaredMutable(declaredMutable),
        name(std::move(name)),
        className(std::move(className)),
        initializer(std::move(initializer)) {}

        void accept(Visitor& visitor) override;
    };

    class ClassExpr : public Expr {
    public:
        std::vector<std::unique_ptr<Stmt>> statements;

        ClassExpr(std::vector<std::unique_ptr<Stmt>> statements):
        statements(std::move(statements)) {}

        void accept(Visitor& visitor) override;
    };

    class ReturnStmt : public Stmt {
    public:
        std::unique_ptr<Expr> expr;

        ReturnStmt(std::unique_ptr<Expr> expr): expr(std::move(expr)) {}

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

    class IfStmt : public Stmt {
        public:
        std::unique_ptr<Expr> condition;
        std::unique_ptr<Stmt> thenBranch;
        std::unique_ptr<Stmt> elseBranch;

        IfStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> thenBranch, std::unique_ptr<Stmt> elseBranch) :
        condition(std::move(condition)),
        thenBranch(std::move(thenBranch)),
        elseBranch(std::move(elseBranch)) {}

        void accept(Visitor& visitor) override;
    };
} // raccoon::compiler::ast