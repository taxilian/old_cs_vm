#pragma once
#ifndef ProcessControlBlock_h__
#define ProcessControlBlock_h__

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <map>
#include "VMCore.h"
#include "HeapMgr.h"
#include "FileHandle.h"

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
    typedef std::map<ProcessState, boost::posix_time::time_duration> StateClock;
    struct ProcessControlBlock {
        static ProcessControlBlockPtr create(int pid, const std::string& name, size_t size, uint32_t offset, uint32_t startAddr, uint32_t heapSize) {
            ProcessControlBlockPtr ptr(boost::make_shared<ProcessControlBlock>());
            ptr->pid = pid;
            ptr->name = name;
            ptr->vm_state.offset = offset;
            ptr->size = size;
            ptr->vm_state.pc = startAddr;
            ptr->procstate = ProcessState_Loading;
			ptr->procMemMgr.setMem(heapSize, offset+size);
            ptr->priority = 0;
            ptr->ptime = boost::posix_time::microsec_clock::local_time();
            ptr->resetMainClock();
            return ptr;
        }
        void resetMainClock() {
            totalTime = boost::posix_time::microsec_clock::local_time();
            ptime = totalTime;
        }
        void setState(const ProcessState state) {
            boost::posix_time::ptime cur(boost::posix_time::microsec_clock::local_time());
            boost::posix_time::time_duration dur(cur-ptime);
            if (stateClock.find(procstate) != stateClock.end()) {
                stateClock[procstate] += dur;
            } else {
                stateClock[procstate] = dur;
            }
            procstate = state;
            // and reset the clock to measure time spent in this state
            ptime = boost::posix_time::microsec_clock::local_time();
        }
        int pid;
        size_t size;
        std::string name;
        VM::VMState vm_state;
        ProcessState procstate;
		HeapMgr procMemMgr;
        int priority;
        boost::posix_time::ptime ptime;
        StateClock stateClock;
        boost::posix_time::ptime totalTime;
        std::map<int, FileHandlePtr> fileTable;
    };
};

#endif // ProcessControlBlock_h__
