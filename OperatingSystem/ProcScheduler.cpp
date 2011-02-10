#include "OpSystem.h"

#include "ProcScheduler.h"

using namespace OS;

ProcScheduler::ProcScheduler( OpSystem* os, VM::VMCore* vm )
    : m_vm(vm), m_os(os), selectedScheduler(ProcScheduler_FirstCome)
{

}

void ProcScheduler::scheduleFirstCome()
{
    if (readyQueue.size()) {
        ProcessControlBlockPtr proc(readyQueue.front());
        proc->procstate = ProcessState_Ready;
        m_vm->setRegisterState(proc->vm_state);
        m_vm->setRunning(true);
    } else {
        m_vm->setRunning(false);
    }
}

ProcScheduler::~ProcScheduler(void)
{
}

void ProcScheduler::scheduleRoundRobin()
{
    
}

void OS::ProcScheduler::addJob( const ProcessControlBlockPtr& job )
{
    readyQueue.push_back(job);
}

void OS::ProcScheduler::findJobs()
{
    readyQueue.clear();
    m_os->getPCBList(readyQueue);
}

void OS::ProcScheduler::schedule()
{
    m_os->saveContext();

    ProcessControlBlockPtr procPtr;
    // First things first; go through and release any jobs that are finished
    ReadyQueue::iterator it = readyQueue.begin();
    while (it != readyQueue.end()) {
        if ((*it)->procstate == ProcessState_Terminating) {
            m_os->freeProcess(*it);
            it = readyQueue.erase(it);
        } else {
            ++it;
        }
    }
    switch(selectedScheduler) {
    case ProcScheduler_FirstCome:
        scheduleFirstCome();
        break;
    case ProcScheduler_RoundRobin:
        scheduleRoundRobin();
        break;
    default:
        break;
    }
}
