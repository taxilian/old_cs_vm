
#pragma once

#include <string>

struct Token
{
    enum TokenType {
        TT_ENDOFFILE,
        TT_SEMICOLON,
        TT_GROUPOPEN,
        TT_GROUPCLOSE,
        TT_NUMBER,
        TT_KEYWORD,
        TT_STRING,
        TT_CHAR,
        TT_WHITESPACE,
        TT_OPERATOR,
        TT_UNKNOWN
    };
    Token() : type(TT_UNKNOWN) {}
    Token(const TokenType type, const std::string& text) : type(type), text(text) {}
    Token& operator=(const Token& rh) {
        type = rh.type;
        text = rh.text;
        return *this;
    }
    TokenType type;
    std::string text;
    std::string toString();
};

