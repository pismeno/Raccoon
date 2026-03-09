#pragma once

#include "Lexer.h"
#include "include/AST.h"
#include <memory>
#include <vector>
#include <stdexcept>

namespace raccoon {

    class ParseError : public std::runtime_error {
    public:
        explicit ParseError(const std::string& message) : std::runtime_error(message) {}
    };

    class Parser {
    public:
        explicit Parser(std::vector<Token> tokens);

        std::unique_ptr<BlockStmt> parse();

    private:
        // Navigation helpers
        bool isAtEnd() const;
        Token peek() const;
        Token previous() const;
        bool check(TokenType type) const;
        bool match(TokenType type);
        Token advance();
        Token consume(TokenType type, const std::string& message);

        // Parsing Logic
        std::unique_ptr<Stmt> declaration();
        std::unique_ptr<Stmt> statement();
        std::unique_ptr<Stmt> varDeclaration();
        std::unique_ptr<Stmt> block();

        std::unique_ptr<Expr> expression();
        std::unique_ptr<Expr> primary();

        ParseError error(Token token, const std::string& message);

        std::vector<Token> tokens;
        int current = 0;
    };

} // namespace raccoon