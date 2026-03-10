#pragma once
#include <string>
#include <vector>

namespace raccoon {

    enum class TokenType {
        LET, BE, EQUAL, NUMBER, IDENTIFIER,
        LPAREN, RPAREN, LBRACE, RBRACE, SEMICOLON, COMMA, EOF_TOKEN,
        RETURN
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