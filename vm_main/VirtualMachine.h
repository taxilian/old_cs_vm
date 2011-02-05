
#pragma once
#ifndef H_VIRTUALMACHINE_RB_
#define H_VIRTUALMACHINE_RB_
#include "VMCore.h"
#include "VMConfig.h"
#include "helpers.h"
#include "Interrupts.h"

namespace VM {
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

    typedef std::map<boost::uint16_t, instructionDef> FunctionMap;

    class VirtualMachine : public VMCore
    {
    public:
        VirtualMachine(void); 
        virtual ~VirtualMachine(void);

        void reset();
        void load(const MemoryBlock& block, boost::uint32_t size);
        Status run();
        void setDebugInfo(std::map<boost::uint32_t, int>& linemap, std::map<boost::uint32_t, std::string> &revLabelMap);
    public:
        // VMCore methods
        Status tick();
        void setMemoryOffset(const uint32_t offset);
        void loadProgram(const MemoryBlock& memory,
                         const size_t size,
                         const uint32_t offset = 0);
        void setRunning( bool isRunning );

        // RegisterState is represented by a list of n values,
        // where v(0) is the PC and v(1...n) are the registers
        VMState getRegisterState();
        void setRegisterState(const VMState& );
        virtual uint32_t getMemorySize();

        virtual void registerInterrupt(int trap, const VM::InterruptHandler& handler);
        // End VMCore methods

    protected:
        VMConfigPtr m_config;
        MemoryBlock m_block;
        bool m_running;
        FunctionMap m_functionMap;
		//For interrupts
		//VM::Interrupts* osInterrupts;
        VM::InterruptTable osInterrupts;
        boost::uint32_t BOUND_CODE;

    public:
        // Registers
        static const int REGISTER_COUNT = 20;
        static const int MEMORY_SIZE = 1024*1024; // 1MB of memory

        boost::int64_t reg[REGISTER_COUNT];
        boost::int64_t pc;
        boost::int32_t offset;
		boost::int32_t pid;

    protected:
        std::map<boost::uint32_t, std::string> labelReverse;
        std::map<boost::uint32_t, int> byteToLineMap;

        std::string getDebugFor(boost::uint32_t addr);
        void registerHandler(std::string name, instructionDef func);
        void callHandler(boost::uint64_t instruction);
        std::string getLabelForAddress(ADDRESS addr);

        // Memory management:
        char get_byte(boost::uint32_t addr);
        void set_byte(boost::uint32_t addr, char value);
        boost::uint64_t get_int(boost::uint32_t addr);
        void set_int(boost::uint32_t addr, boost::uint64_t value);

        void writeThreadRegisters(int threadId);
        void loadThreadRegisters( int newThreadId );

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
}

#endif
