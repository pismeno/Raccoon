#include "../include/Lexer.hpp"
#include <cctype>

namespace raccoon::compiler {

    Lexer::Lexer(std::string source) : source(std::move(source)) {}

    std::vector<Token> Lexer::tokenize() {
        std::vector<Token> tokens;
        cursor = 0;

        while (cursor < source.length()) {
            char c = source[cursor];

            if (isspace(c)) {
                cursor++;
                continue;
            }

            if (c == '=') { tokens.push_back({TokenType::EQUAL, "="}); cursor++; continue; }
            if (c == '+') { tokens.push_back({TokenType::PLUS, "+"}); cursor++; continue; }
            if (c == '-') { tokens.push_back({TokenType::MINUS, "-"}); cursor++; continue; }
            if (c == '*') { tokens.push_back({TokenType::TIMES, "*"}); cursor++; continue; }
            if (c == '/') { tokens.push_back({TokenType::SLASH, "/"}); cursor++; continue; }
            if (c == '(') { tokens.push_back({TokenType::LPAREN, "("}); cursor++; continue; }
            if (c == ')') { tokens.push_back({TokenType::RPAREN, ")"}); cursor++; continue; }
            if (c == '{') { tokens.push_back({TokenType::LBRACE, "{"}); cursor++; continue; }
            if (c == '}') { tokens.push_back({TokenType::RBRACE, "}"}); cursor++; continue; }
            if (c == ';') { tokens.push_back({TokenType::SEMICOLON, ";"}); cursor++; continue; }
            if (c == ',') { tokens.push_back({TokenType::COMMA, ","}); cursor++; continue; }

            if (isalpha(c)) {
                std::string word = "";
                while (cursor < source.length() && isalnum(source[cursor])) {
                    word += source[cursor++];
                }

                if (word == "den") tokens.push_back({TokenType::DEN, word});
                else if (word == "let") tokens.push_back({TokenType::LET, word});
                else if (word == "be") tokens.push_back({TokenType::BE, word});
                else if (word == "return") tokens.push_back({TokenType::RETURN, word});
                else tokens.push_back({TokenType::IDENTIFIER, word});
                continue;
            }

            if (isdigit(c)) {
                std::string num = "";
                while (cursor < source.length() && isdigit(source[cursor])) {
                    num += source[cursor++];
                }
                tokens.push_back({TokenType::NUMBER, num});
                continue;
            }

            cursor++;
        }

        tokens.push_back({TokenType::EOF_TOKEN, ""});
        return tokens;
    }
}