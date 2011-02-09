#include "OpSystem.h"

#include "ProcScheduler.h"

using namespace OS;

ProcScheduler::ProcScheduler( OpSystem* os, VM::VMCore* vm )
    : m_vm(vm), m_os(os)
{

}


ProcScheduler::~ProcScheduler(void)
{
}

void ProcScheduler::scheduleRoundRobin()
{
    readyQueue.clear();
    m_os->getPCBList(readyQueue);
}
