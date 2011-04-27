
#pragma once
#ifndef H_VIRTUALMACHINE_RB_
#define H_VIRTUALMACHINE_RB_
#include "VMCore.h"
#include "VMConfig.h"
#include "helpers.h"
#include <boost\date_time\posix_time\posix_time.hpp>

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
	
    union intsplit {
        uint64_t iVal;
        struct {
            uint8_t cb1[8];
        } cVal;
    };
	
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
        bool isRunning();
        void setRunning( bool isRunning );

        // RegisterState is represented by a list of n values,
        // where v(0) is the PC and v(1...n) are the registers
        VMState getRegisterState();
        void setRegisterState(const VMState& );
        virtual uint32_t getMemorySize();

        virtual void registerInterrupt(int trap, const VM::InterruptHandler& handler);
        virtual void configureScheduler( const int baseTicks, const double variance, const InterruptHandler& interrupt);
		virtual void setPageFault(const boost::function<void (int)>& interrupt);//assumes that pageNeeded has been set by VM.
        virtual void resetRunningTime() { runningTime *= 0; }
        virtual boost::posix_time::time_duration getRunningTime() { return runningTime; }
        virtual uint64_t getAvgMemTimeHit() { return memTime_hit.total_microseconds() / memHitCount; }
        virtual uint64_t getAvgMemTimeMiss() { return memTime_miss.total_microseconds() / memMissCount; }
        virtual void readMemory(const uint32_t addr, MemoryBlock& memory, size_t size);
        virtual void writeMemory(const uint32_t addr, const char* memory, size_t size);
		virtual void readFrame(const int frame, MemoryBlock& memory);
		virtual void writeFrame(const int frame, const int page, const char* memory);
        virtual int whichPage(int frame);
        // End VMCore methods

        // returns true if a page fault was needed
        bool mapVirtualMemory(uint64_t vaddr, uint64_t& physaddr);

    protected:
        VMConfigPtr m_config;
        MemoryBlock m_block;
        bool m_running;
        FunctionMap m_functionMap;
		//For CPU Utitilization stats.
		boost::posix_time::time_duration runningTime;

        uint64_t memHitCount;
        uint64_t memMissCount;
        boost::posix_time::time_duration memTime_hit;
        boost::posix_time::time_duration memTime_miss;
		//For interrupts
		//VM::Interrupts* osInterrupts;
        VM::InterruptTable osInterrupts;
        boost::uint32_t BOUND_CODE;

        int sched_calcTarget();
        int sched_baseTicks;
        double sched_variance;
        InterruptHandler sched_interrupt;
		boost::function<void (int)> page_fault;//when a page fault occurs just call this handler, assumes that pageNeeded has been set by VM.

    public:
        // Registers
        static const int REGISTER_COUNT = 20;

        int pageTable[FRAME_COUNT];

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
