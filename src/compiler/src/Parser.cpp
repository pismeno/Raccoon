#include "../include/Parser.hpp"

namespace raccoon::compiler {

    using namespace ast;

    Parser::Parser(std::vector<Token> tokens) : tokens(std::move(tokens)) {}

    std::unique_ptr<BlockStmt> Parser::parse() {
        std::vector<std::unique_ptr<Stmt>> statements;
        while (!isAtEnd()) {
            statements.push_back(declaration());
        }
        return std::make_unique<BlockStmt>(std::move(statements));
    }


    bool Parser::isAtEnd() const { return peek().type == TokenType::EOF_TOKEN; }
    Token Parser::peek() const { return tokens[current]; }
    Token Parser::next() const { return tokens[current + 1]; }
    Token Parser::previous() const { return tokens[current - 1]; }
    bool Parser::check(TokenType type) const { return !isAtEnd() && peek().type == type; }

    Token Parser::peek(int distance) const {
        if (current + distance >= tokens.size()) return tokens.back();
        return tokens[current + distance];
    }

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


    std::unique_ptr<Stmt> Parser::declaration() {
        if (match(TokenType::LET)) return varDeclaration();
        if (match(TokenType::DEN)) return denDeclaration();
        return statement();
    }

    std::unique_ptr<Stmt> Parser::statement() {
        if (match(TokenType::LBRACE)) return block();
        if (match(TokenType::RETURN)) return ret();

        if (check(TokenType::IDENTIFIER)) {
            if (next().type == TokenType::ASSIGN) {
                return varAssignment();
            }

            return expressionStatement();
        }

        throw error(peek(), "Expected statement or declaration.");
    }

    std::unique_ptr<Stmt> Parser::expressionStatement() {
        std::unique_ptr<Expr> expr = expression();
        consume(TokenType::SEMICOLON, "Expected ';' after expression.");
        return std::make_unique<ExprStmt>(std::move(expr));
    }

    std::unique_ptr<Stmt> Parser::denDeclaration() {
        Token name = consume(TokenType::IDENTIFIER, "Expected den name.");
        consume(TokenType::LBRACE, "Expected '{' for den body.");
        std::vector<std::unique_ptr<Stmt>> contents;
        while (!check(TokenType::RBRACE) && !isAtEnd()) {
            contents.push_back(declaration());
        }
        consume(TokenType::RBRACE, "Expect '}' after den body.");

        consume(TokenType::SEMICOLON, "Expected ';' after den declaration.");

        return std::make_unique<DenStmt>(name.lexeme, std::move(contents));
    }

    std::unique_ptr<Stmt> Parser::varDeclaration() {
        Token name = consume(TokenType::IDENTIFIER, "Expected variable name.");
        bool isMutable = false;
        consume(TokenType::BE, "Expected 'be' after variable name.");

        if (match(TokenType::MUTABLE)) {
            isMutable = true;
        }

        if (check(TokenType::LPAREN)) {
            return functionDeclaration(name, isMutable);
        }

        Token type = consume(TokenType::IDENTIFIER, "Expected variable type.");

        std::unique_ptr<Expr> initializer = nullptr;
        if (match(TokenType::ASSIGN)) {
            initializer = expression();
        }

        consume(TokenType::SEMICOLON, "Expected ';' after variable declaration.");
        return std::make_unique<VariableDecl>(name.lexeme, isMutable, type.lexeme, std::move(initializer));
    }

    std::unique_ptr<Stmt> Parser::varAssignment() {
        Token name = consume(TokenType::IDENTIFIER, "Expected variable name.");
        consume(TokenType::ASSIGN, "Expected '=' after variable name.");
        std::unique_ptr<Expr> value = expression();
        consume(TokenType::SEMICOLON, "Expected ';' after variable assignment.");
        return std::make_unique<VariableAssign>(name.lexeme, std::move(value));
    }

    std::unique_ptr<Stmt> Parser::functionDeclaration(const Token name, bool isMutable) {
        consume(TokenType::LPAREN, "Expected '(' for function type parameters.");
        std::vector<std::string> paramTypes;
        if (!check(TokenType::RPAREN)) {
            do {
                paramTypes.push_back(consume(TokenType::IDENTIFIER, "Expected parameter type/name.").lexeme);
            } while (match(TokenType::COMMA));
        }
        consume(TokenType::RPAREN, "Expected ')' after function type parameters.");

        Token returnType = consume(TokenType::IDENTIFIER, "Expected return type.");

        std::unique_ptr<Expr> initializer = nullptr;
        if (match(TokenType::ASSIGN)) {
            initializer = expression();
        }

        consume(TokenType::SEMICOLON, "Expected ';' after function declaration.");
        return std::make_unique<FunctionDecl>(name.lexeme, isMutable, paramTypes, returnType.lexeme, std::move(initializer));
    }

    std::unique_ptr<BlockStmt> Parser::block() {
        std::vector<std::unique_ptr<Stmt>> statements;
        while (!check(TokenType::RBRACE) && !isAtEnd()) {
            statements.push_back(declaration());
        }
        consume(TokenType::RBRACE, "Expect '}' after block.");
        return std::make_unique<BlockStmt>(std::move(statements));
    }

    std::unique_ptr<Expr> Parser::expression() {
        if (isFuncExpression()) {
            return funcExpression();
        }
        return logicalOr();
    }

    std::unique_ptr<Expr> Parser::funcExpression() {
        consume(TokenType::LPAREN, "Expected '(' at start of function.");

        std::vector<std::string> paramNames;
        if (!check(TokenType::RPAREN)) {
            do {
                paramNames.push_back(consume(TokenType::IDENTIFIER, "Expected parameter name.").lexeme);
            } while (match(TokenType::COMMA));
        }
        consume(TokenType::RPAREN, "Expected ')' after parameters.");

        consume(TokenType::LBRACE, "Expected '{' before function body.");
        std::unique_ptr<BlockStmt> body = block();

        return std::make_unique<FunctionExpr>(paramNames, std::move(body));
    }

    bool Parser::isFuncExpression() {
        if (!check(TokenType::LPAREN)) return false;

        int i = 1;
        int signatureCursor = 1;

        while (signatureCursor > 0 && (current + i) < tokens.size()) {
            if (peek(i).type == TokenType::LPAREN) signatureCursor++;
            else if (peek(i).type == TokenType::RPAREN) signatureCursor--;
            i++;
        }

        if ((current + i) >= tokens.size()) return false;

        return peek(i).type == TokenType::LBRACE;
    }

    std::unique_ptr<Expr> Parser::finishCall(const std::string& name) {
        std::vector<std::unique_ptr<Expr>> arguments;
        if (!check(TokenType::RPAREN)) {
            do {
                arguments.push_back(expression());
            } while (match(TokenType::COMMA));
        }
        consume(TokenType::RPAREN, "Expected ')' after arguments.");
        return std::make_unique<CallExpr>(name, std::move(arguments));
    }

    std::unique_ptr<Stmt> Parser::ret() {
        std::unique_ptr<Expr> expr = expression();
        consume(TokenType::SEMICOLON, "Expected ';' after return statement.");
        return std::make_unique<ReturnStmt>(std::move(expr));
    }

    std::unique_ptr<Expr> Parser::logicalOr() {
        auto left = logicalAnd();
        while (match(TokenType::OR)) {
            std::string op = previous().lexeme;
            auto right = logicalAnd();
            left = std::make_unique<BinaryExpression>(op, std::move(left), std::move(right));
        }
        return left;
    }

    std::unique_ptr<Expr> Parser::logicalAnd() {
        auto left = comparison();
        while (match(TokenType::AND)) {
            std::string op = previous().lexeme;
            auto right = comparison();
            left = std::make_unique<BinaryExpression>(op, std::move(left), std::move(right));
        }
        return left;
    }

    std::unique_ptr<Expr> Parser::comparison() {
        auto left = term();
        while (match(TokenType::LESSER) || match(TokenType::GREATER) ||
               match(TokenType::LESSER_EQUAL) || match(TokenType::GREATER_EQUAL) ||
               match(TokenType::EQUAL) || match(TokenType::NOT_EQUAL)) {
            std::string op = previous().lexeme;
            auto right = term();
            left = std::make_unique<BinaryExpression>(op, std::move(left), std::move(right));
        }
        return left;
    }

    std::unique_ptr<Expr> Parser::unary() {
        if (match(TokenType::MINUS) || match(TokenType::NOT)) {
            std::string op = previous().lexeme;
            std::unique_ptr<Expr> right = unary();
            return std::make_unique<UnaryExpression>(op, std::move(right));
        }

        return primary();
    }

    std::unique_ptr<Expr> Parser::term() {
        std::unique_ptr<Expr> left = factor();

        while (match(TokenType::PLUS) || match(TokenType::MINUS)) {
            std::string op = previous().lexeme;
            std::unique_ptr<Expr> right = factor();
            left = std::make_unique<BinaryExpression>(op, std::move(left), std::move(right));
        }

        return left;
    }

    std::unique_ptr<Expr> Parser::factor() {
        std::unique_ptr<Expr> left = unary();

        while (match(TokenType::TIMES) || match(TokenType::SLASH)) {
            std::string op = previous().lexeme;
            std::unique_ptr<Expr> right = unary();
            left = std::make_unique<BinaryExpression>(op, std::move(left), std::move(right));
        }

        return left;
    }

    std::unique_ptr<Expr> Parser::primary() {
        if (match(TokenType::NUMBER) || match(TokenType::TRUE) || match(TokenType::FALSE)) {
            if (previous().type == TokenType::TRUE) return std::make_unique<LiteralExpr>(true);
            if (previous().type == TokenType::FALSE) return std::make_unique<LiteralExpr>(false);
            return std::make_unique<LiteralExpr>(std::stoi(previous().lexeme));
        }

        if (match(TokenType::IDENTIFIER)) {
            std::string name = previous().lexeme;

            if (match(TokenType::LPAREN)) {
                return finishCall(name);
            }

            return std::make_unique<VariableExpr>(name);
        }

        if (match(TokenType::LPAREN)) {
            std::unique_ptr<Expr> expr = expression();
            consume(TokenType::RPAREN, "Expected ')' after expression.");
            return expr;
        }

        throw error(peek(), "Expected expression.");
    }

    ParseError Parser::error(Token token, const std::string& message) {
        return ParseError("[Line " /*+ std::to_string(token.line) +*/ "] Error at '" + token.lexeme + "': " + message);
    }

} // namespace raccoon::compiler