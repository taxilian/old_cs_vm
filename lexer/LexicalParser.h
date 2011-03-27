#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <deque>
#include <map>
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

    Token& current();
    Token nextToken();
    Token peekToken(size_t n = 0);
    bool backTrack(int n = 1);
    int getLineNumber() { return m_lineNo; }
    std::string getLine(int num) { return m_lineCache[num]; }

private:
    Token _nextToken();

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
    Token endToken( const TokenType type, const bool back_up = false);
    bool isValidIdentifier( const char c, const bool isFirstChar = false );
    bool nextChar(char& c);
private:
    std::string buffer;
    int startLinePos;
    int bufferpos;
    std::map<int, std::string> m_lineCache;
    std::string m_filename;
    std::ifstream m_file;
    StateEnum m_state;
    unsigned int m_lineNo;
    std::deque<Token> fwd_q;
    std::deque<Token> back_q;

    std::stringstream ss;
};

