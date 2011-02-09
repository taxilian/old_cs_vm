#pragma once
#ifndef VMCore_h__
#define VMCore_h__

// Virtual Machine interface file

#include <boost/noncopyable.hpp>
#include <boost/shared_array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>
#include <boost/function.hpp>
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
		//process currently running
		int32_t pid; 
    };

    class VMCore : boost::noncopyable
    {
    public:
        static const int FP = 16;
        static const int SL = 17;
        static const int SP = 18;
        static const int SB = 19;
    public:
        virtual Status run() = 0;

        virtual void setMemoryOffset(const uint32_t offset) = 0;
        virtual void loadProgram(const MemoryBlock& memory,
                                 const size_t size,
                                 const uint32_t offset = 0) = 0;

        virtual VMState getRegisterState() = 0;
        virtual void setRegisterState(const VMState& ) = 0;

        virtual uint32_t getMemorySize() = 0;

        virtual void registerInterrupt(int trap, const InterruptHandler& handler) = 0;

        virtual bool isRunning() = 0;
        virtual void setRunning( bool isRunning ) = 0;
        virtual void configureScheduler( const int baseTicks, const double variance, const InterruptHandler& interrupt) = 0;
    };

};
#endif // VMCore_h__