#pragma once
#ifndef OpSystem_h__
#define OpSystem_h__

#include <string>
#include <list>
#include <boost/noncopyable.hpp>

#include "ProcessControlBlock.h"
#include "VMCore.h"
#include "HeapMgr.h"

namespace OS {
    class OpSystem : boost::noncopyable 
    {
    public:
        OpSystem(VM::VMCore* vm);
        ~OpSystem(void);

        void load(const std::string& pathfileName,const std::string& name);
        int getNextPid() { return m_lastPid++; }
        void ps() const;
        void run(int pid);
		void mem();
		void mem(int pid);
		void free();
		void free(int pid);
		//interrupts
		void processNew(VM::VMCore* vm);
		void processFree(VM::VMCore* vm);
		void processYield(VM::VMCore* vm);

        void runScheduler(VM::VMCore* vm);
    protected:
        ProcessControlBlockPtr getProcess(int pid);

    private:
        static const int stackSize = 4096;
		static const int heapSize = 1024;
        uint32_t m_lastLoadAddr;
        VM::VMCore* m_vm;
        std::list<ProcessControlBlockPtr> m_processList;
        int m_lastPid;
		HeapMgr sysMemMgr;
    };
}
#endif // OpSystem_h__
