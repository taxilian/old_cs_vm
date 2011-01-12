#pragma once
#ifndef VMCore_h__
#define VMCore_h__

// Virtual Machine interface file

#include <boost/noncopyable.hpp>
#include <boost/shared_array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>
#include <list>

using boost::uint64_t;
using boost::int64_t;
using boost::uint32_t;
using boost::int32_t;
using boost::uint16_t;
using boost::int16_t;
using boost::uint8_t;
using boost::int8_t;

namespace VM {
    typedef boost::uint32_t ADDRESS;
    typedef boost::int32_t IMMEDIATE;
    typedef boost::int64_t REGISTER;

    typedef boost::shared_array<uint8_t> MemoryBlock;
    typedef std::list<REGISTER> RegisterList;

    enum Status {
        Status_Idle,
        Status_Stopped,
        Status_Running
    };

    class VMCore : boost::noncopyable
    {
    public:
        virtual Status tick() = 0;

        virtual void setMemoryOffset(const uint32_t offset) = 0;
        virtual void loadProgram(const MemoryBlock& memory,
                                 const size_t size,
                                 const uint32_t offset = 0,
                                 const size_t stackSize = 0) = 0;

        virtual RegisterList getRegisterState() = 0;
        virtual void setRegisterState(const RegisterList& ) = 0;

        virtual uint32_t getMemorySize() = 0;
    };

};
#endif // VMCore_h__