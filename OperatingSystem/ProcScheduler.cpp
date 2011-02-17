#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

#include "OpSystem.h"
#include "ProcScheduler.h"

using namespace OS;
using namespace boost::posix_time;

ProcScheduler::ProcScheduler( OpSystem* os, VM::VMCore* vm )
    : m_vm(vm), m_os(os), selectedScheduler(ProcScheduler_FirstCome),
    startTimeCount(0), endTimeCount(0), waitTimeCount(0)
{

}

ProcScheduler::~ProcScheduler(void)
{
}

void ProcScheduler::scheduleFirstCome()
{
    if (readyQueue.size()) {
        ProcessControlBlockPtr proc(readyQueue.front());
        setProcActive(proc);
    } else {
        m_vm->setRunning(false);
    }
}

bool sortByPriority(const ProcessControlBlockPtr& a, const ProcessControlBlockPtr& b)
{
    return a->priority > b->priority;
}
bool sortBySize(const ProcessControlBlockPtr& a, const ProcessControlBlockPtr& b)
{
    return a->size < b->size;
}

void OS::ProcScheduler::schedulePriority()
{
    if (readyQueue.size()) {
        // Just like round robin, but sorted each time by priority
        std::stable_sort(readyQueue.begin(), readyQueue.end(), boost::lambda::bind(&sortByPriority, boost::lambda::_1, boost::lambda::_2));
        ProcessControlBlockPtr proc(readyQueue.front());
        setProcActive(proc);
        
        // Move the entry to the back
        readyQueue.push_back(proc);
        readyQueue.pop_front();
    } else {
        m_vm->setRunning(false);
    }
}

void OS::ProcScheduler::scheduleAdvanced()
{//shortest job first scheduling
	if (readyQueue.size()) {
        // Just like first come, but sorted each time by priority
        std::stable_sort(readyQueue.begin(), readyQueue.end(), boost::lambda::bind(&sortBySize, boost::lambda::_1, boost::lambda::_2));
        ProcessControlBlockPtr proc(readyQueue.front());
        setProcActive(proc);
        
        // Move the entry to the back
        readyQueue.push_back(proc);
        readyQueue.pop_front();
    } else {
        m_vm->setRunning(false);
    }
}

void ProcScheduler::scheduleRoundRobin()
{
    if (readyQueue.size()) {
        // Take the next entry in the queue and set it to run
        ProcessControlBlockPtr proc(readyQueue.front());
        setProcActive(proc);
        
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
            buildStats(*it);
            m_os->freeProcess(*it);
            it = readyQueue.erase(it);
        } else if ((*it)->procstate == ProcessState_Ready) {
            // If there are any running processes, suspend them
            (*it)->setState(ProcessState_Suspended);
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
    case ProcScheduler_Priority:
        schedulePriority();
        break;
	case ProcScheduler_Advanced:
		scheduleAdvanced();
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

void OS::ProcScheduler::setProcActive( ProcessControlBlockPtr proc )
{
    proc->setState(ProcessState_Ready);
    m_vm->setRegisterState(proc->vm_state);
    m_vm->setRunning(true);
}

void OS::ProcScheduler::buildStats( const ProcessControlBlockPtr& proc )
{
    ptime cur(microsec_clock::local_time());
    // Add to running average of start time
    this->startTime += proc->stateClock[ProcessState_Ready];
    this->startTimeCount++;

    // Add to running average of end time
    this->endTime += (cur-proc->totalTime);
    this->endTimeCount++;

    // Add to running average of wait time
    this->waitTime += proc->stateClock[ProcessState_Ready] + proc->stateClock[ProcessState_Suspended];
    this->waitTimeCount++;
}

void OS::ProcScheduler::printStats()
{
    time_duration resp(startTime / startTimeCount);
    std::cout << "Average response time: " << resp.total_microseconds() << "us" << std::endl;
    time_duration turn(endTime / endTimeCount);
    std::cout << "Average turnaround time: " << turn.total_microseconds() << "us" << std::endl;
    time_duration wait(waitTime / waitTimeCount);
    std::cout << "Average wait time: " << wait.total_microseconds() << "us" << std::endl;
}

void OS::ProcScheduler::resetStats()
{
    startTime *= 0;
    endTime = waitTime = startTime;
    startTimeCount = endTimeCount = waitTimeCount = 0;
}
