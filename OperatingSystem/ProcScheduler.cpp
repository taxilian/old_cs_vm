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
    if (readyQueue.size()) {
        // Take the next entry in the queue and set it to run
        ProcessControlBlockPtr proc(readyQueue.front());
        proc->procstate = ProcessState_Ready;
        m_vm->setRegisterState(proc->vm_state);
        m_vm->setRunning(true);
        
        // Move the entry to the back
        readyQueue.push_back(proc);
        readyQueue.pop_front();
    } else {
        m_vm->setRunning(false);
    }
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
        } else if ((*it)->procstate == ProcessState_Ready) {
            // If there are any running processes, suspend them
            (*it)->procstate = ProcessState_Suspended;
            ++it;
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

void OS::ProcScheduler::printAlgorithm()
{
    std::cout << "Selected scheduling algorithm: ";
    switch(selectedScheduler) {
    case ProcScheduler_FirstCome:
        std::cout << "First come first served" << std::endl;
        break;
    case ProcScheduler_RoundRobin:
        std::cout << "Round Robin" << std::endl;
        break;
    case ProcScheduler_Priority:
        std::cout << "Priority" << std::endl;
        break;
    case ProcScheduler_Advanced:
        std::cout << "Advanced algorithm" << std::endl;
        break;
    default:
        std::cout << "Err... none?" << std::endl;
        break;
    }
}
