#pragma once

#include "Lexer.hpp"
#include "ast/AST.hpp"
#include <memory>
#include <vector>
#include <stdexcept>

namespace raccoon::compiler {

    using namespace ast;

    class ParseError : public std::runtime_error {
    public:
        explicit ParseError(const std::string& message) : std::runtime_error(message) {}
    };

    class Parser {
    public:
        explicit Parser(std::vector<Token> tokens);

        std::unique_ptr<BlockStmt> parse();

    private:
        bool isAtEnd() const;
        Token peek() const;
        Token previous() const;
        bool check(TokenType type) const;
        bool match(TokenType type);
        Token advance();
        Token consume(TokenType type, const std::string& message);

        std::unique_ptr<Stmt> declaration();
        std::unique_ptr<Stmt> statement();
        std::unique_ptr<Stmt> denDeclaration();
        std::unique_ptr<Stmt> varDeclaration();
        std::unique_ptr<Stmt> functionDeclaration(Token name, bool isMutable);
        std::unique_ptr<BlockStmt> block();

        std::unique_ptr<Expr> expression();
        std::unique_ptr<Expr> primary();
        std::unique_ptr<Expr> term();
        std::unique_ptr<Expr> factor();
        std::unique_ptr<Expr> unary();

        ParseError error(Token token, const std::string& message);

        std::vector<Token> tokens;
        int current = 0;
    };

} // namespace raccoon