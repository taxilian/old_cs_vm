#pragma once
#ifndef ProcessControlBlock_h__
#define ProcessControlBlock_h__

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include "VMCore.h"

namespace OS {
    enum ProcessState {
        ProcessState_Ready,
        ProcessState_Waiting,
        ProcessState_Suspended,
        ProcessState_Loading,
        ProcessState_Terminating,
    };

    struct ProcessControlBlock;
    typedef boost::shared_ptr<ProcessControlBlock> ProcessControlBlockPtr;
    struct ProcessControlBlock {
        static ProcessControlBlockPtr create(int pid, const std::string& name, size_t size, uint32_t offset, uint32_t startAddr) {
            ProcessControlBlockPtr ptr(boost::make_shared<ProcessControlBlock>());
            ptr->pid = pid;
            ptr->name = name;
            ptr->vm_state.offset = offset;
            ptr->size = size;
            ptr->vm_state.pc = startAddr;
            ptr->procstate = ProcessState_Loading;
            return ptr;
        }
        int pid;
        size_t size;
        std::string name;
        VM::VMState vm_state;
        ProcessState procstate;
    };
};

#endif // ProcessControlBlock_h__