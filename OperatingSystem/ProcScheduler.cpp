#include "OpSystem.h"

#include "ProcScheduler.h"

using namespace OS;

ProcScheduler::ProcScheduler( OpSystem* os, VM::VMCore* vm )
    : m_vm(vm), m_os(os)
{

}

void ProcScheduler::scheduleFirstCome()
{

    if (readyQueue.front()->procstate == ProcessState_Ready) {

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
