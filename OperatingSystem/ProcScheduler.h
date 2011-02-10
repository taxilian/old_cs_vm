#pragma once

#include <deque>
#include "VMCore.h"
#include "ProcessControlBlock.h"

namespace OS {
    enum ProcSchedulerType {
        ProcScheduler_RoundRobin,
        ProcScheduler_FirstCome,
        ProcScheduler_Priority,
        ProcScheduler_Advanced
    };

    class OpSystem;

    class ProcScheduler
    {
    public:
        ProcScheduler(OpSystem* os, VM::VMCore* vm);
        ~ProcScheduler(void);

        void setScheduler(const ProcSchedulerType sched) { selectedScheduler = sched; }

        void addJob(const ProcessControlBlockPtr& job);
        void findJobs();
        void schedule();
        void printAlgorithm();

    protected:
        void scheduleRoundRobin();
        void scheduleFirstCome();
        void schedulePriority();
        void scheduleAdvanced();
    private:
        OpSystem* m_os;
        VM::VMCore* m_vm;
        ProcSchedulerType selectedScheduler;

        typedef std::deque<ProcessControlBlockPtr> ReadyQueue;
        ReadyQueue readyQueue;
    };
};
