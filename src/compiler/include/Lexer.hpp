#pragma once
#include <string>
#include <vector>

namespace raccoon::compiler {

    enum class TokenType {
        LET, BE, MUTABLE,
        DEN, DOT,
        ASSIGN,
        TIMES, PLUS, MINUS, SLASH,
        GREATER, LESSER, GREATER_EQUAL, LESSER_EQUAL, EQUAL, NOT_EQUAL, AND, OR, NOT,
        NUMBER, IDENTIFIER, TRUE, FALSE, CLASS, OBJECT,
        LPAREN, RPAREN, LBRACE, RBRACE,
        SEMICOLON, COMMA,
        RETURN, IF, ELSE,
        EOF_TOKEN
    };

    struct Token {
        TokenType type;
        std::string lexeme;
    };

    class Lexer {
    private:
        std::string source;
        int cursor = 0;

    public:
        explicit Lexer(std::string source);

        std::vector<Token> tokenize();
    };
} // namespace raccoon::compiler