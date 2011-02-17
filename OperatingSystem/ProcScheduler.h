#pragma once

#include <boost/date_time/posix_time/posix_time.hpp>
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
        void printStats();

    protected:
        void scheduleRoundRobin();
        void scheduleFirstCome();
        void schedulePriority();

        void setProcActive( ProcessControlBlockPtr proc );

        void scheduleAdvanced();
        void buildStats( const ProcessControlBlockPtr& proc );
        void resetStats();
    private:
        OpSystem* m_os;
        VM::VMCore* m_vm;
        ProcSchedulerType selectedScheduler;

        typedef std::deque<ProcessControlBlockPtr> ReadyQueue;
        ReadyQueue readyQueue;

        boost::posix_time::time_duration startTime;
        int startTimeCount;

        boost::posix_time::time_duration endTime;
        int endTimeCount;

        boost::posix_time::time_duration waitTime;
        int waitTimeCount;

    };
};
