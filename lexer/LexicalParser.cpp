
#include <iostream>
#include <exception>

#include "LexicalParser.h"

LexicalParser::LexicalParser(const std::string& filename)
    : m_filename(filename), m_file(filename), m_lineNo(0), m_state(STATE_EMPTY)
{
    if (m_file.bad())
        throw std::exception(("Could not load " + m_filename).c_str());
}

LexicalParser::~LexicalParser(void)
{
}

Token LexicalParser::nextToken()
{
    char c(0);
    while (m_file.get(c)) {
        if (c == '\n')
            ++m_lineNo;
        switch(getState()) {
        case STATE_EMPTY:
            // Nothing accumulated
            if (c == ';') {
                return Token(Token::TT_SEMICOLON, ";");
            } else if (isGroupOpen(c)) {
                return Token(Token::TT_GROUPOPEN, std::string(&c, 1));
            } else if (isGroupClose(c)) {
                return Token(Token::TT_GROUPCLOSE, std::string(&c, 1));
            } else if (isWhitespace(c)) {
                return Token(Token::TT_WHITESPACE, std::string(&c, 1));
            } else if (isNumeric(c)) {
                setState(STATE_NUMBER);
                ss << c;
            } else if (isOperator(c)) {
                setState(STATE_OPERATOR);
                ss << c;
            } else if (c == '\'') {
                setState(STATE_SQUOTE);
            } else if (c == '"') {
                setState(STATE_DQUOTE);
            } else if (isValidIdentifier(c, true)) {
                setState(STATE_IDENTIFIER);
                ss << c;
            } else {
                return Token(Token::TT_UNKNOWN, std::string(&c, 1));
            }
            break;
        case STATE_IDENTIFIER:
            if (isValidIdentifier(c)) {
                ss << c;
            } else {
                return endToken(Token::TT_KEYWORD, true);
            }
            break;
        case STATE_NUMBER:
            if (isNumeric(c)) {
                ss << c;
            } else {
                // If not numeric, end of token
                return endToken(Token::TT_NUMBER, true);
            }
            break;
        case STATE_OPERATOR:
            if (isOperator(c)) {
                ss << c;
            } else {
                // If not an operator, end of token
                return endToken(Token::TT_OPERATOR, true);
            }
            if (ss.str() == "//") {
                setState(STATE_COMMENT);
                ss.str("");
            }
            break;
        case STATE_COMMENT:
            if (c == '\n') {
                setState(STATE_EMPTY);
            }
            break;
        case STATE_SQUOTE:
            if (c == '\\') {
                if (!m_file.get(c)) {
                    throw LexicalParserException("Unmatched ' quote! Unexpected End of File!");
                }
                if (c == 'r')
                    ss << '\r';
                else if (c == 'n')
                    ss << '\n';
                else if (c == 't')
                    ss << '\t';
            } else if (c == '\'') {
                return endToken(Token::TT_CHAR);
            } else {
                ss << c;
            }
            break;
        case STATE_DQUOTE:
            if (c == '\\') {
                if (!m_file.get(c)) {
                    throw LexicalParserException("Unmatched \" quote! Unexpected End of File!");
                }
                if (c == 'r')
                    ss << '\r';
                else if (c == 'n')
                    ss << '\n';
                else if (c == 't')
                    ss << '\t';
            } else if (c == '"') {
                return endToken(Token::TT_STRING);
            } else if (c == '\n') {
                std::cerr << "Unmatched \" in string on line " << (m_lineNo - 1);
                return endToken(Token::TT_STRING);
            } else {
                ss << c;
            }
            break;
        }
    }
    return Token(Token::TT_ENDOFFILE, "");
}

Token LexicalParser::endToken(const Token::TokenType type, const bool back_up)
{
    setState(STATE_EMPTY);

    if (back_up) {
        std::streamoff pos(m_file.tellg());
        --pos;
        m_file.seekg(pos);
    }

    Token out(type, ss.str());
    ss.str("");
    return out;
}

bool LexicalParser::isNumeric( const char c )
{
    return (c >= '0' && c <= '9');
}

bool LexicalParser::isWhitespace( const char c )
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

bool LexicalParser::isGroupOpen( const char c )
{
    switch(c) {
    case '(':
    case '{':
    case '[':
        return true;
    default:
        return false;
    }
}

bool LexicalParser::isGroupClose( const char c )
{
    switch(c) {
    case ')':
    case '}':
    case ']':
        return true;
    default:
        return false;
    }
}

bool LexicalParser::isOperator( const char c )
{
    switch(c) {
    case '+':
    case '-':
    case '/':
    case '*':
    case '=':
    case '&':
    case '|':
    case '^':
    case '!':
    case '<':
    case '>':
    case '.':
    case ',':
    case ':':
        return true;
    default:
        return false;
    }
}

bool LexicalParser::isValidIdentifier( const char c, const bool isFirstChar )
{
    if (c >= 'a' && c <= 'z') {
        return true;
    } else if (c >= 'A' && c <= 'Z') {
        return true;
    } else if (c >= '0' && c <= '9' && !isFirstChar) {
        return true;
    } else if (c == '_') {
        return true;
    } else {
        return false;
    }
}
