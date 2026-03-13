#pragma once
#include <string>
#include <vector>

namespace raccoon::compiler {

    enum class TokenType {
        LET, BE, MUTABLE,
        DEN,
        EQUAL, TIMES, PLUS, MINUS, SLASH,
        NUMBER, IDENTIFIER,
        LPAREN, RPAREN, LBRACE, RBRACE,
        SEMICOLON, COMMA,
        RETURN,
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
}