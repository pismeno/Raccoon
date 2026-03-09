#include "Parser.h"

namespace raccoon {

    Parser::Parser(std::vector<Token> tokens) : tokens(std::move(tokens)) {}

    std::unique_ptr<BlockStmt> Parser::parse() {
        std::vector<std::unique_ptr<Stmt>> statements;
        while (!isAtEnd()) {
            statements.push_back(declaration());
        }
        return std::make_unique<BlockStmt>(std::move(statements));
    }

// --- Navigation ---

    bool Parser::isAtEnd() const { return peek().type == TokenType::EOF_TOKEN; }
    Token Parser::peek() const { return tokens[current]; }
    Token Parser::previous() const { return tokens[current - 1]; }
    bool Parser::check(TokenType type) const { return !isAtEnd() && peek().type == type; }

    bool Parser::match(TokenType type) {
        if (check(type)) {
            advance();
            return true;
        }
        return false;
    }

    Token Parser::advance() {
        if (!isAtEnd()) current++;
        return previous();
    }

    Token Parser::consume(TokenType type, const std::string& message) {
        if (check(type)) return advance();
        throw error(peek(), message);
    }

// --- Parsing ---

    std::unique_ptr<Stmt> Parser::declaration() {
        if (match(TokenType::LET)) return varDeclaration();
        return statement();
    }

    std::unique_ptr<Stmt> Parser::statement() {
        if (match(TokenType::LBRACE)) return block();

        // If it's not a special statement, we assume it's an expression
        // For now, we'll just throw an error if it doesn't match anything
        throw error(peek(), "Expected statement or declaration.");
    }

    std::unique_ptr<Stmt> Parser::varDeclaration() {
        Token name = consume(TokenType::IDENTIFIER, "Expected variable name.");
        consume(TokenType::BE, "Expected 'be' after variable name.");

        // Check for optional 'mut'
        //bool isMutable = match(TokenType::MUT);

        Token type = consume(TokenType::IDENTIFIER, "Expected variable type.");

        std::unique_ptr<Expr> initializer = nullptr;
        if (match(TokenType::EQUAL)) {
            initializer = expression();
        }

        consume(TokenType::SEMICOLON, "Expected ';' after variable declaration.");
        return std::make_unique<VariableDecl>(name.lexeme, type.lexeme, std::move(initializer));
    }

    std::unique_ptr<Stmt> Parser::block() {
        std::vector<std::unique_ptr<Stmt>> statements;
        while (!check(TokenType::RBRACE) && !isAtEnd()) {
            statements.push_back(declaration());
        }
        consume(TokenType::RBRACE, "Expect '}' after block.");
        return std::make_unique<BlockStmt>(std::move(statements));
    }

    std::unique_ptr<Expr> Parser::expression() {
        return primary();
    }

    std::unique_ptr<Expr> Parser::primary() {
        if (match(TokenType::NUMBER)) {
            std::string text = previous().lexeme;
            try {
                int64_t value = std::stoll(text);
                return std::make_unique<LiteralExpr>(value);
            } catch (...) {
                throw error(previous(), "Number too large: " + text);
            }
        }

        throw error(peek(), "Expected expression.");
    }

    ParseError Parser::error(Token token, const std::string& message) {
        return ParseError("[Line " /*+ std::to_string(token.line) +*/ "] Error at '" + token.lexeme + "': " + message);
    }

} // namespace raccoon