/**
 * Parser.h
 *
 * Richard Bateman
 * Virtual Machine Assembly parser
 */

#pragma once
#ifndef H_PARSER
#define H_PARSER

#include <string>
#include <vector>
#include <fstream>
#include <deque>
#include <boost/shared_ptr.hpp>
#include <boost/tokenizer.hpp>

#include "VMConfig.h"

typedef boost::tokenizer< boost::escaped_list_separator<char> > Tokenizer;

struct ParserException : std::exception
{
    ParserException(const std::string& error)
        : m_error(error)
    { }
    ~ParserException() throw() { }
    virtual const char* what() const throw() { 
        return m_error.c_str(); 
    }
    std::string m_error;
};

class Parser
{
public:
    struct Line {
        virtual ~Line() { };
        std::string label;
    };

    struct Byte : public Line {
        unsigned char value;
    };
    struct Int : public Line {
        int value;
    };

    struct Instruction : public Line {
        std::string name;
        std::vector<std::string> args;
    };

    typedef boost::shared_ptr<Line> LinePtr;
    typedef boost::shared_ptr<Byte> BytePtr;
    typedef boost::shared_ptr<Int> IntPtr;
    typedef boost::shared_ptr<Instruction> InstructionPtr;
public:
    Parser(std::string filename, VMConfigPtr config);
    ~Parser(void);

protected:
    std::string sanitizeString(const std::string &str);
    std::vector<std::string> split(const std::string &str, const char *tokens);

public:
    void processFile();
    LinePtr getNextLine();
    std::streamsize getLineNumber() { return m_lineNumber; }

protected:
    std::streamsize m_lineNumber;
    std::ifstream m_file;
    std::deque<LinePtr> m_queue;
    VMConfigPtr m_config;
    bool end;
};

#endif
