#pragma once
#ifndef OpSystem_h__
#define OpSystem_h__

#include <string>
#include <list>

#include "ProcessControlBlock.h"
#include "VMCore.h"

namespace OS {
    class ProcScheduler;
    class OpSystem
    {
    public:
        OpSystem(VM::VMCore* vm);
        ~OpSystem(void);

        void load(const std::string& fileName);
        int getNextPid() { return m_lastPid++; }
        void ps() const;
        void run(int pid);

    protected:
        ProcessControlBlockPtr getProcess(int pid);
        template <class CONT>
        void getPCBList(CONT& container) {
            for (std::list<ProcessControlBlockPtr>::const_iterator it = m_processList.begin();
                it != m_processList.end(); ++it) {
                container.push_back(*it);
            }
        }

    private:
        static const int stackSize = 4096;
        uint32_t m_lastLoadAddr;
        VM::VMCore* m_vm;
        std::list<ProcessControlBlockPtr> m_processList;
        int m_lastPid;

        friend class ProcScheduler;
    };
}
#endif // OpSystem_h__
