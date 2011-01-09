#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include "Token.h"

struct LexicalParserException : std::exception
{
    LexicalParserException(const std::string& error)
        : m_error(error)
    { }
    ~LexicalParserException() throw() { }
    virtual const char* what() const throw() { 
        return m_error.c_str(); 
    }
    std::string m_error;
};

class LexicalParser
{
public:
    LexicalParser(const std::string& filename);
    ~LexicalParser(void);

    Token nextToken();

protected:
    enum StateEnum {
        STATE_EMPTY,
        STATE_SQUOTE,
        STATE_DQUOTE,
        STATE_NUMBER,
        STATE_OPERATOR,
        STATE_IDENTIFIER,
        STATE_COMMENT
    };
    StateEnum getState() const { return m_state; }
    void setState(const StateEnum nstate) { m_state = nstate; }

    static bool isWhitespace(const char c);
    static bool isNumeric(const char c);
    static bool isGroupOpen(const char c);
    static bool isGroupClose(const char c);
    bool isOperator( const char c );
    Token endToken( const Token::TokenType type, const bool back_up = false);
    bool isValidIdentifier( const char c, const bool isFirstChar = false );
private:
    std::string m_filename;
    std::ifstream m_file;
    StateEnum m_state;
    unsigned int m_lineNo;

    std::stringstream ss;
};

