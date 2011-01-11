
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

struct thread
{
    bool active;
    ADDRESS MEMB;
    ADDRESS SB;
    ADDRESS SL;
};

typedef std::map<boost::uint16_t, instructionDef> FunctionMap;

class VirtualMachine
{
public:
    VirtualMachine(void);
    virtual ~VirtualMachine(void);

    void reset();
    void load(boost::shared_array<unsigned char> block, boost::uint32_t size);
    void run(boost::uint32_t start);
    void setDebugInfo(std::map<boost::uint32_t, int>& linemap, std::map<boost::uint32_t, std::string> &revLabelMap);

protected:
    VMConfigPtr m_config;
    boost::shared_array<unsigned char> m_block;
    bool m_running;
    FunctionMap m_functionMap;
    thread threadList[20];
    int m_curThread;
    int threadCount;
    bool m_blocked;

    boost::uint32_t BOUND_CODE;

public:
    static const int THREAD_NUM = 20;

    static const int FP = 16;
    static const int SL = 17;
    static const int SP = 18;
    static const int SB = 19;
    boost::int64_t reg[THREAD_NUM];
    boost::int64_t pc;

protected:
    std::map<boost::uint32_t, std::string> labelReverse;
    std::map<boost::uint32_t, int> byteToLineMap;
    std::string getDebugFor(boost::uint32_t addr);
    void registerHandler(std::string name, instructionDef func);
    void callHandler(boost::uint64_t instruction);
    char get_byte(boost::uint32_t addr);
    void set_byte(boost::uint32_t addr, char value);
    boost::uint64_t get_int(boost::uint32_t addr);
    void set_int(boost::uint32_t addr, boost::uint64_t value);
    std::string getLabelForAddress(ADDRESS addr);

    void initThread( int threadId, ADDRESS newPC );
    void endThread( int threadId );
    void changeContext( int newThreadId );
    void writeThreadRegisters(int threadId);
    void loadThreadRegisters( int newThreadId );
    void switchToNextThread();
    int getNextAvailableThread();

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

    void RUN(REGISTER &rd, ADDRESS addr);
    void END();
    void BLK();
    void LCK(ADDRESS addr);
    void ULK(ADDRESS addr);
};

#endif
