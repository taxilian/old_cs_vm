
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

class VirtualMachine
{
public:
public:
    VirtualMachine(void);
    virtual ~VirtualMachine(void);

    void reset();
    void load(boost::shared_array<unsigned int> block);
    void run(unsigned short start);
    void setDebugInfo(std::map<unsigned short, int>& map) { byteToLineMap = map; }

protected:
    VMConfigPtr m_config;
    boost::shared_array<unsigned int> m_block;
    bool m_running;
    std::map<unsigned char, CallInstructionFunctor> m_functionMap;

public:
    unsigned int reg[16];
    unsigned int pc;

protected:
    std::map<unsigned short, int> byteToLineMap;
    std::string getDebugFor(unsigned short addr);
    void registerHandler(unsigned char inst, CallInstructionFunctor func);
    void callHandler(unsigned int instruction);

protected:
    void JMP(ADDRESS addr);
    void JMR(REGISTER &r);
    void BRZ(REGISTER &r, ADDRESS addr);
    void BNZ(REGISTER &r, ADDRESS addr);
    void BLT(REGISTER &r, ADDRESS addr);
    void BGT(REGISTER &r, ADDRESS addr);

    void LD(REGISTER &rd, REGISTER &rsp);
    void LDA(REGISTER &rd, ADDRESS addr);
    void STR(REGISTER &rs, ADDRESS addr);
    void LDR(REGISTER &rd, ADDRESS addr);
    void LDB(REGISTER &rd, ADDRESS addr);
    void STB(REGISTER &rs, ADDRESS addr);
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
