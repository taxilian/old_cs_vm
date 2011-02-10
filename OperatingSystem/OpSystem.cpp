#include "HexWriter.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/cast.hpp>

#include "ProcScheduler.h"
#include "OpSystem.h"

using namespace OS;
using VM::VMCore;

OpSystem::OpSystem(VM::VMCore* vm) : m_lastLoadAddr(0), m_vm(vm), m_lastPid(1),sysMemMgr(m_vm->getMemorySize(), 0),
    m_scheduler(new ProcScheduler(this, vm))
{
    // Register any special traps here
    vm->registerInterrupt(0, boost::bind(&OpSystem::processEnd, this, _1));
    vm->registerInterrupt(5, boost::bind(&OpSystem::processNew, this, _1));
    vm->registerInterrupt(6, boost::bind(&OpSystem::processFree, this, _1));
    vm->registerInterrupt(7, boost::bind(&OpSystem::processYield, this, _1));

    vm->configureScheduler(50, 0.5, boost::bind(&OpSystem::runScheduler, this, _1));
}


OpSystem::~OpSystem(void)
{
}

void printPCB(const ProcessControlBlockPtr& pcb) {
    std::cout << std::setiosflags(std::ios::left);
    std::cout << std::setw(5) << pcb->pid;
    std::cout << std::setw(3) << ' ';
    std::cout << std::setw(20) << pcb->name;
	std::cout << std::setw(3) << ' ';
	std::cout << std::setw(10) << pcb->vm_state.reg[VMCore::SB];
	std::cout << std::setw(3) << ' ';
	std::cout << std::setw(10) << pcb->vm_state.reg[VMCore::SL];
	std::cout << std::setw(3) << ' ';
	std::cout << std::setw(8) << pcb->priority;
    std::cout << std::endl;
}

void OS::OpSystem::ps() const
{
    using namespace boost::lambda;
    std::cout << std::setiosflags(std::ios::left);
    std::cout << std::setw(5) << "P ID";
    std::cout << std::setw(3) << ' ';
    std::cout << std::setw(20) << "Process Name";
	std::cout << std::setw(3) << ' ';
	std::cout << std::setw(10) << "Base reg";
	std::cout << std::setw(3) << ' ';
	std::cout << std::setw(10) << "Limit reg";
	std::cout << std::setw(3) << ' ';
    std::cout << std::setw(8) << "Priority";
    std::cout << std::endl;
    std::for_each(m_processList.begin(), m_processList.end(),
        boost::lambda::bind(&printPCB, boost::lambda::_1));
}

void OS::OpSystem::load( const std::string& pathfileName,const std::string& name)
{
	VM::MemoryBlock memory;
    size_t memorySize;
    uint32_t startAddress;
    uint32_t offset;
    VM::ReadFromHex(pathfileName, memory, memorySize, startAddress, offset);
   // offset = m_lastLoadAddr;
    //m_lastLoadAddr += memorySize + stackSize;
	offset = sysMemMgr.allocate(memorySize + stackSize + heapSize);
    m_vm->loadProgram(memory, memorySize, offset);
    ProcessControlBlockPtr newProgram(ProcessControlBlock::create(getNextPid(), name, memorySize, offset, startAddress, heapSize));
    m_processList.push_back(newProgram);

    // Set up the stack
    newProgram->vm_state.reg[VMCore::SB] = 
        newProgram->vm_state.reg[VMCore::SP] = 
        newProgram->vm_state.reg[VMCore::FP] = 
            offset + memorySize + stackSize;
	newProgram->vm_state.pid = newProgram->pid;
    //newProgram->vm_state.reg[VMCore::SL] = offset + memorySize;
	newProgram->vm_state.reg[VMCore::SL] = offset + memorySize + heapSize;
    newProgram->procstate = ProcessState_Loading;
    std::cout << "Loaded " << name << " in pid " << newProgram->pid << std::endl;
    m_scheduler->addJob(newProgram);
}

ProcessControlBlockPtr OS::OpSystem::getProcess(int pid)
{
    using namespace boost::lambda;
    std::list<ProcessControlBlockPtr>::iterator it =
        std::find_if(m_processList.begin(), m_processList.end(),
            boost::lambda::bind(&ProcessControlBlock::pid, boost::lambda::bind(&ProcessControlBlockPtr::operator*, boost::lambda::_1)) == boost::lambda::var(pid));
    if (it == m_processList.end())
        return ProcessControlBlockPtr();
    else
        return *it;
}

void OS::OpSystem::run(int pid)
{
    ProcessControlBlockPtr ptr(getProcess(pid));
    if (ptr) {
        m_vm->setRegisterState(ptr->vm_state);
        ptr->procstate = ProcessState_Ready;
        m_vm->run();
		if(ptr->procstate == ProcessState_Suspended)
		{
			ptr->vm_state = m_vm->getRegisterState();
		}
		else
		{
			ptr->procstate = ProcessState_Terminating;
			sysMemMgr.de_allocate(ptr->vm_state.offset);
            m_processList.remove(ptr);
		}
	} 
	else {
        std::stringstream ss;
        ss << "Could not load process with pid: " << pid;
        throw std::runtime_error(ss.str());
    }
}

void OS::OpSystem::runall()
{
    do {
        m_vm->run();
        m_scheduler->schedule();
    } while (m_vm->isRunning());
}

void OpSystem::mem()
{
	sysMemMgr.display();
}
void OpSystem::mem(int pid)
{
	ProcessControlBlockPtr ptr(getProcess(pid));
	ptr->procMemMgr.display();
}
void OpSystem::free()
{
	std::cout << "System free memory: " << sysMemMgr.freeMemAmount() << " bytes\n";
}
void OpSystem::free(int pid)
{
	ProcessControlBlockPtr ptr(getProcess(pid));
	std::cout <<"Process free memory: " << ptr->procMemMgr.freeMemAmount() << " bytes\n";
}

void OS::OpSystem::processNew(VM::VMCore* vm)
{//interrupt 5
	VM::VMState temp = vm->getRegisterState();
    size_t size = boost::numeric_cast<size_t>(temp.reg[0]);
    
	ProcessControlBlockPtr ptr(getProcess(temp.pid));
	VM::ADDRESS addr = ptr->procMemMgr.allocate(size);
    temp.reg[1] = addr;
    vm->setRegisterState(temp);
}

void OS::OpSystem::processFree(VM::VMCore* vm)
{//interrupt 6
	VM::VMState temp = vm->getRegisterState();
	ProcessControlBlockPtr ptr(getProcess(temp.pid));
    VM::ADDRESS addr = boost::numeric_cast<VM::ADDRESS>(temp.reg[0]);
	ptr->procMemMgr.de_allocate(addr);
}

void OS::OpSystem::processYield(VM::VMCore* vm)
{//interrupt 7
	VM::VMState temp = vm->getRegisterState();
	ProcessControlBlockPtr ptr(getProcess(temp.pid));
	ptr->procstate = ProcessState_Suspended;
    ptr->vm_state = temp;
    vm->setRunning(false);
}

void OS::OpSystem::processEnd( VM::VMCore* vm )
{// interrupt 0 / TRP 0
	VM::VMState temp = vm->getRegisterState();
	ProcessControlBlockPtr ptr(getProcess(temp.pid));
	ptr->procstate = ProcessState_Terminating;
    ptr->vm_state = temp;
    vm->setRunning(false);
}

void OS::OpSystem::saveContext()
{
    VM::VMState temp = m_vm->getRegisterState();
    ProcessControlBlockPtr ptr(getProcess(temp.pid));
    if (ptr)
        ptr->vm_state = temp;
}

void OS::OpSystem::freeProcess( const ProcessControlBlockPtr& proc )
{
    proc->procstate = ProcessState_Terminating;
    sysMemMgr.de_allocate(proc->vm_state.offset);
    m_processList.remove(proc);
}

void OS::OpSystem::setAlgorithm( int alg_no )
{
    switch(alg_no) {
    default:
    case 1:
        m_scheduler->setScheduler(ProcScheduler_FirstCome);
        break;
    case 2:
        m_scheduler->setScheduler(ProcScheduler_RoundRobin);
        break;
    case 3:
        m_scheduler->setScheduler(ProcScheduler_Priority);
        break;
    case 4:
        m_scheduler->setScheduler(ProcScheduler_Advanced);
        break;
    }
    m_scheduler->printAlgorithm();
}

void OS::OpSystem::printAlgorithm()
{
    m_scheduler->printAlgorithm();
}

void OS::OpSystem::runScheduler( VM::VMCore* vm )
{
    m_scheduler->schedule();
}

void OS::OpSystem::setPriority( int pid, int priority )
{
    ProcessControlBlockPtr proc(getProcess(pid));
    if (proc)
        proc->priority = priority;
    else
        std::cout << "Error: Could not find process #" << pid << std::endl;
}
