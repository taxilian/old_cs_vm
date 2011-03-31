
#include <iostream>
#include <exception>
#include <boost/algorithm/string.hpp>

#include "LexicalParser.h"

LexicalParser::LexicalParser(const std::string& filename)
    : m_filename(filename), m_file(filename.c_str()), m_lineNo(1), m_state(STATE_EMPTY), bufferpos(0), startLinePos(0)
{
    if (!m_file)
        throw LexicalParserException(("Could not load " + m_filename).c_str());
    char c;
    std::stringstream data;
    while (m_file.get(c)) {
        data << c;
    }
    buffer = data.str();
    boost::algorithm::erase_all(buffer, "\r");
}

LexicalParser::~LexicalParser(void)
{
}

Token LexicalParser::nextToken()
{
    Token t;
    if (fwd_q.size() > 0) {
        t = fwd_q.front();
        fwd_q.pop_front();
    } else {
        t = _nextToken();
    }
    back_q.push_front(t);
    return t;
}

Token& LexicalParser::current()
{
    return back_q.front();
}

Token LexicalParser::peekToken( size_t n /*= 0*/ )
{
    while (fwd_q.size() < n + 1) {
        fwd_q.push_back(_nextToken());
    }
    return fwd_q.at(n);
}

bool LexicalParser::backTrack( int n /*= 1*/ )
{
    if (back_q.size() >= static_cast<size_t>(n)) {
        for (int i = 0; i < n; i++) {
            fwd_q.push_front(back_q.front());
            back_q.pop_front();
        }
        return true;
    } else {
        return false;
    }
}

bool LexicalParser::nextChar(char& c)
{
    if (bufferpos >= buffer.size()-1 || buffer[bufferpos] == '\0')
        return false;
    if (buffer[bufferpos] == '\n') {
        m_lineNo++;
        char* beg = &buffer[bufferpos+1];
        if (*beg) {
            char* end = strchr(beg, '\n');
            std::string line(beg, (end-beg));
            m_lineCache[m_lineNo] = line;
        }
    } else if (bufferpos == 0) {
        char* beg = &buffer[0];
        char* end = strchr(beg, '\n');
        std::string line(beg, (end-beg));
        m_lineCache[m_lineNo] = line;
    }
    c = buffer[bufferpos++];
    return true;
}

Token LexicalParser::_nextToken()
{
    char c(0);
    while (nextChar(c)) {
        switch(getState()) {
        case STATE_EMPTY:
            // Nothing accumulated
            if (c == ';') {
                return Token(TT_SEMICOLON, ";");
            } else if (isGroupOpen(c)) {
                return Token(TT_GROUPOPEN, std::string(&c, 1));
            } else if (isGroupClose(c)) {
                return Token(TT_GROUPCLOSE, std::string(&c, 1));
            } else if (isWhitespace(c)) {
                continue;
                //return Token(TT_WHITESPACE, std::string(&c, 1));
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
                return Token(TT_UNKNOWN, std::string(&c, 1));
            }
            break;
        case STATE_IDENTIFIER:
            if (isValidIdentifier(c)) {
                ss << c;
            } else {
                return endToken(TT_KEYWORD, true);
            }
            break;
        case STATE_NUMBER:
            if (isNumeric(c)) {
                ss << c;
            } else {
                // If not numeric, end of token
                return endToken(TT_NUMBER, true);
            }
            break;
        case STATE_OPERATOR:
            if (isOperator(c)) {
                ss << c;
            } else {
                // If not an operator, end of token
                return endToken(TT_OPERATOR, true);
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
                if (!nextChar(c)) {
                    throw LexicalParserException("Unmatched ' quote! Unexpected End of File!");
                }
                if (c == 'r')
                    ss << '\r';
                else if (c == 'n')
                    ss << '\n';
                else if (c == 't')
                    ss << '\t';
            } else if (c == '\'') {
                return endToken(TT_CHAR);
            } else {
                ss << c;
            }
            break;
        case STATE_DQUOTE:
            if (c == '\\') {
                if (!nextChar(c)) {
                    throw LexicalParserException("Unmatched \" quote! Unexpected End of File!");
                }
                if (c == 'r')
                    ss << '\r';
                else if (c == 'n')
                    ss << '\n';
                else if (c == 't')
                    ss << '\t';
            } else if (c == '"') {
                return endToken(TT_STRING);
            } else if (c == '\n') {
                std::cerr << "Unmatched \" in string on line " << m_lineNo;
                return endToken(TT_STRING);
            } else {
                ss << c;
            }
            break;
        }
    }
    return Token(TT_ENDOFFILE, "");
}

Token LexicalParser::endToken(const TokenType type, const bool back_up)
{
    setState(STATE_EMPTY);

    if (back_up) {
        --bufferpos;
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
