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
	struct PTEntry  {
		int pageNum;    //page number for a given process id.
		bool valid;     //true page is in physical memory. false page is in disk.
		PTEntry(){pageNum = -1; valid = false;};
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
		int currentFrame;               // we are replacing the frame after currentFrame. if currentFrame=7 we replace frame 0.
		std::list<PTEntry> PageTable;   // available to VM but managed by OS.
		std::list<PTEntry> framesInfo;  // We are replacing the frame after the current frame being executed.
		// if current frame is 7 then we go back to raplacing frame 0. 
		// framesInfo contains the pages information about pages currently in physical memory.
		// 8 pages of 512 bytes. need to populate this container when first loading into physical memory.
		PTEntry pageNeeded;             // When calling OpSystem.getPage(VM::VMCore* vm) need to set this field.
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
		virtual void pageFault(const InterruptHandler& interrupt) = 0; // assumes that pageNeeded has been set by VM.
		virtual void readFrame(const uint32_t addr, MemoryBlock& memory, size_t size) = 0;
        virtual void writeFrame(const uint32_t addr, const char* memory, size_t size) = 0;
    };

};
#endif // VMCore_h__