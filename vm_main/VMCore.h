#pragma once
#ifndef VMCore_h__
#define VMCore_h__

// Virtual Machine interface file

#include <boost/noncopyable.hpp>
#include <boost/shared_array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>
#include <boost/function.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <list>
#include <map>

using boost::uint64_t;
using boost::int64_t;
using boost::uint32_t;
using boost::int32_t;
using boost::uint16_t;
using boost::int16_t;
using boost::uint8_t;
using boost::int8_t;

namespace VM {
    class VMCore;
    typedef boost::uint32_t ADDRESS;
    typedef boost::int32_t IMMEDIATE;
    typedef boost::int64_t REGISTER;

    typedef boost::function<void (VMCore*)> InterruptHandler;
    typedef std::map<int, InterruptHandler> InterruptTable;

    typedef boost::shared_array<uint8_t> MemoryBlock;
    typedef std::list<REGISTER> RegisterList;

    enum Status {
        Status_Idle,
        Status_Stopped,
        Status_Running
    };

    struct VMState {
        uint64_t pc;
        int32_t offset;
        int64_t reg[20];
		// process currently running
		int32_t pid; 
    };
    class VMCore : boost::noncopyable
    {
    public:
        static const int FP = 16;
        static const int SL = 17;
        static const int SP = 18;
        static const int SB = 19;
		static const int PTSize = 32;   // 32 pages of virtual memory.
		static const int FMSize = 8;    // number of frames in physical memory.
        static const int FRAME_SIZE = 512;
        static const int FRAME_COUNT = 8;
        static const int MEMORY_SIZE = FRAME_SIZE * FRAME_COUNT;

        static const int VIRTUAL_MEM_SIZE = 1024*1024;
    public:
        virtual Status run() = 0;

        virtual void setMemoryOffset(const uint32_t offset) = 0;
        virtual void loadProgram(const MemoryBlock& memory,
                                 const size_t size,
                                 const uint32_t offset = 0) = 0;

        virtual VMState getRegisterState() = 0;
        virtual void setRegisterState(const VMState& ) = 0;

        virtual uint32_t getMemorySize() = 0;
        virtual void readMemory(const uint32_t addr, MemoryBlock& memory, size_t size) = 0;
        virtual void writeMemory(const uint32_t addr, const char* memory, size_t size) = 0;

        virtual void registerInterrupt(int trap, const InterruptHandler& handler) = 0;

        virtual bool isRunning() = 0;
        virtual void setRunning( bool isRunning ) = 0;
        virtual void configureScheduler( const int baseTicks, const double variance, const InterruptHandler& interrupt) = 0;
		virtual void resetRunningTime() = 0;
        virtual boost::posix_time::time_duration getRunningTime() = 0;
		// virtual memory
		virtual void setPageFault(const boost::function<void (int)>& interrupt) = 0; // assumes that pageNeeded has been set by VM.
		virtual void readFrame(const int frame, MemoryBlock& memory) = 0;
		virtual void writeFrame(const int frame, const int page, const char* memory) = 0;
        virtual int whichPage(int frame) = 0;
        virtual uint64_t getAvgMemTimeHit() = 0;
        virtual uint64_t getAvgMemTimeMiss() = 0;
    };

};
#endif // VMCore_h__