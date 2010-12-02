/**
 * Assembler.h
 *
 * Richard Bateman
 * Virtual Machine Assembler
 */

#pragma once
#ifndef H_ASSEMBLER
#define H_ASSEMBLER

#include <string>
#include <boost/shared_array.hpp>
#include "Parser.h"


struct AssemblerException : std::exception
{
    AssemblerException(const std::string& error)
        : m_error(error)
    { }
    ~AssemblerException() throw() { }
    virtual const char* what() const throw() { 
        return m_error.c_str(); 
    }
    std::string m_error;
};

class Assembler
{
public:
    Assembler(std::string filename);
    ~Assembler(void);
    unsigned short start();
    boost::shared_array<unsigned char> getBlock() { return m_block; }

    std::map<unsigned short, int> byteToLineMap;

protected:
    char getRegister(std::string name);

protected:
    VMConfigPtr m_config;
    Parser m_parser;
    boost::shared_array<unsigned char> m_block;
};

#endif
