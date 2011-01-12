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
    boost::uint32_t start();
    boost::shared_array<unsigned char> getBlock() { return m_block; }
    boost::uint32_t getBlockSize() { return m_size; }

    std::map<boost::uint32_t, int> byteToLineMap;
    std::map<boost::uint32_t, std::string> labelReverse;

protected:
    char getRegister(std::string name);
protected:
    VMConfigPtr m_config;
    Parser m_parser;
    boost::shared_array<unsigned char> m_block;
    boost::uint32_t m_size;
};

#endif
