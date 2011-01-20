#include "HexWriter.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>


#include "OpSystem.h"

using namespace OS;
using VM::VMCore;

OpSystem::OpSystem(VM::VMCore* vm) : m_lastLoadAddr(0), m_vm(vm), m_lastPid(1)
{
}


OpSystem::~OpSystem(void)
{
}

void printPCB(const ProcessControlBlockPtr& pcb) {
    std::cout << std::setiosflags(std::ios::left);
    std::cout << std::setw(5) << pcb->pid;
    std::cout << std::setw(3) << ' ';
    std::cout << std::setw(60) << pcb->name;
    std::cout << std::endl;
}

void OS::OpSystem::ps() const
{
    using namespace boost::lambda;
    std::cout << std::setiosflags(std::ios::left);
    std::cout << std::setw(5) << "P ID";
    std::cout << std::setw(3) << ' ';
    std::cout << std::setw(60) << "Process Name";
    std::cout << std::endl;
    std::for_each(m_processList.begin(), m_processList.end(),
        boost::lambda::bind(&printPCB, boost::lambda::_1));
}

void OS::OpSystem::load( const std::string& fileName )
{
    VM::MemoryBlock memory;
    size_t memorySize;
    uint32_t startAddress;
    uint32_t offset;
    VM::ReadFromHex(fileName, memory, memorySize, startAddress, offset);
    offset = m_lastLoadAddr;
    m_lastLoadAddr += memorySize + stackSize;

    m_vm->loadProgram(memory, memorySize, offset);
    ProcessControlBlockPtr newProgram(ProcessControlBlock::create(getNextPid(), fileName, memorySize, offset, startAddress));
    m_processList.push_back(newProgram);

    // Set up the stack
    newProgram->vm_state.reg[VMCore::SB] = 
        newProgram->vm_state.reg[VMCore::SP] = 
        newProgram->vm_state.reg[VMCore::FP] = 
            offset + memorySize + stackSize;
    newProgram->vm_state.reg[VMCore::SL] = offset + memorySize;

    newProgram->procstate = ProcessState_Waiting;
    std::cout << "Loaded " << fileName << " in pid " << newProgram->pid << std::endl;
}

ProcessControlBlockPtr OS::OpSystem::getProcess(int pid)
{
    for (std::list<ProcessControlBlockPtr>::const_iterator it = m_processList.begin();
        it != m_processList.end(); ++it) {
        if ((*it)->pid == pid)
            return *it;
    }
    return ProcessControlBlockPtr();
}

void OS::OpSystem::run(int pid)
{
    ProcessControlBlockPtr ptr(getProcess(pid));
    if (ptr) {
        m_vm->setRegisterState(ptr->vm_state);
        ptr->procstate = ProcessState_Ready;
        m_vm->run();
        ptr->procstate = ProcessState_Terminating;
    } else {
        std::stringstream ss;
        ss << "Could not load process with pid: " << pid;
        throw new std::runtime_error(ss.str());
    }
}