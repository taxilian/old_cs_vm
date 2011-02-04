#pragma once
#ifndef OpSystem_h__
#define OpSystem_h__

#include <string>
#include <list>

#include "ProcessControlBlock.h"
#include "VMCore.h"
#include "HeapMgr.h"
#include "Interrupts.h"

namespace OS {
    class OpSystem : public VM::Interrupts 
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
		int sysNew(int size);
		void sysDelete(int addr);
		void yield();
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
