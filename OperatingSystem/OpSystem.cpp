#include "HexWriter.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>


#include "OpSystem.h"

using namespace OS;
using VM::VMCore;

OpSystem::OpSystem(VM::VMCore* vm) : m_lastLoadAddr(0), m_vm(vm), m_lastPid(1),sysMemMgr(m_vm->getMemorySize(), 0)
{
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
    newProgram->procstate = ProcessState_Ready;
    std::cout << "Loaded " << name << " in pid " << newProgram->pid << std::endl;
}

ProcessControlBlockPtr OS::OpSystem::getProcess(int pid)
{
    using namespace boost::lambda;
    std::list<ProcessControlBlockPtr>::iterator it =
        std::find_if(m_processList.begin(), m_processList.end(),
            bind(&ProcessControlBlock::pid, bind(&ProcessControlBlockPtr::operator*, _1)) == var(pid));
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
			using namespace boost::lambda;
			std::list<ProcessControlBlockPtr>::iterator it =
			std::find_if(m_processList.begin(), m_processList.end(),
				bind(&ProcessControlBlock::pid, bind(&ProcessControlBlockPtr::operator*, _1)) == var(pid));
			m_processList.erase(it);
		}
	} 
	else {
        std::stringstream ss;
        ss << "Could not load process with pid: " << pid;
        throw std::runtime_error(ss.str());
    }
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
int OS::OpSystem::sysNew(int size)
{//interrupt 5
	VM::VMState temp = m_vm->getRegisterState();
	ProcessControlBlockPtr ptr(getProcess(temp.pid));
	return ptr->procMemMgr.allocate(size); 
}
void OS::OpSystem::sysDelete(int addr)
{//interrupt 6
	VM::VMState temp = m_vm->getRegisterState();
	ProcessControlBlockPtr ptr(getProcess(temp.pid));
	return ptr->procMemMgr.de_allocate(addr);
}
void OS::OpSystem::yield()
{//interrupt 7
	VM::VMState temp = m_vm->getRegisterState();
	ProcessControlBlockPtr ptr(getProcess(temp.pid));
	ptr->procstate = ProcessState_Suspended;
}