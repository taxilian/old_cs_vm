
#pragma once
#ifndef H_VIRTUALMACHINE_RB_
#define H_VIRTUALMACHINE_RB_

#include <boost/shared_array.hpp>
#include "VMConfig.h"
#include "helpers.h"

struct VMException : std::exception
{
    VMException(const std::string& error)
        : m_error(error)
    { }
    ~VMException() throw() { }
    virtual const char* what() const throw() { 
        return m_error.c_str(); 
    }
    std::string m_error;
};

typedef std::map<unsigned char, instructionDef> FunctionMap;

class VirtualMachine
{
public:
public:
    VirtualMachine(void);
    virtual ~VirtualMachine(void);

    void reset();
    void load(boost::shared_array<unsigned char> block);
    void run(unsigned short start);
    void setDebugInfo(std::map<unsigned short, int>& linemap, std::map<unsigned short, std::string> &revLabelMap);

protected:
    VMConfigPtr m_config;
    boost::shared_array<unsigned char> m_block;
    bool m_running;
    FunctionMap m_functionMap;

public:
    int reg[16];
    int pc;

protected:
    std::map<unsigned short, std::string> labelReverse;
    std::map<unsigned short, int> byteToLineMap;
    std::string getDebugFor(unsigned short addr);
    void registerHandler(std::string name, instructionDef func);
    void callHandler(unsigned int instruction);
    char get_byte(unsigned short addr);
    void set_byte(unsigned short addr, char value);
    int get_int(unsigned short addr);
    void set_int(unsigned short addr, int value);

protected:
    void JMP(ADDRESS addr);
    void JMR(REGISTER &r);
    void BRZ(REGISTER &r, ADDRESS addr);
    void BNZ(REGISTER &r, ADDRESS addr);
    void BLT(REGISTER &r, ADDRESS addr);
    void BGT(REGISTER &r, ADDRESS addr);

    void LDR(REGISTER &rd, ADDRESS addr);
    void LDR2(REGISTER &rd, REGISTER &rs);
    void LDA(REGISTER &rd, ADDRESS addr);
    void STR(REGISTER &rs, ADDRESS addr);
    void STR2(REGISTER &rs, REGISTER &rd);
    void LDB(REGISTER &rd, ADDRESS addr);
    void LDB2(REGISTER &rd, REGISTER &rs);
    void STB(REGISTER &rs, ADDRESS addr);
    void STB2(REGISTER &rs, REGISTER &rd);
    void MOV(REGISTER &rd, REGISTER &rs);

    void TRP(IMMEDIATE i);

    void ADI(REGISTER &rd, IMMEDIATE val);
    void ADD(REGISTER &rd, REGISTER &rs);
    void SUB(REGISTER &rd, REGISTER &rs);
    void MUL(REGISTER &rd, REGISTER &rs);
    void DIV(REGISTER &rd, REGISTER &rs);

    void AND(REGISTER &rd, REGISTER &rs);
    void OR(REGISTER &rd, REGISTER &rs);
    void CMP(REGISTER &rd, REGISTER &rs);
};

#endif
