#include "HexWriter.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "VirtualFileHandle.h"
#include "ProcScheduler.h"
#include "OpSystem.h"

using namespace OS;
using namespace boost::posix_time;
using VM::VMCore;

OpSystem::OpSystem(VM::VMCore* vm, VM::VirtualDisk* vd) :
    m_lastLoadAddr(0), m_vm(vm), fileSystem(vd), m_lastPid(1),sysMemMgr(m_vm->getMemorySize(), 0),
    m_scheduler(new ProcScheduler(this, vm)), cwd(0), lastFH(0)
{
    // Register any special traps here
    vm->registerInterrupt(0, boost::bind(&OpSystem::processEnd, this, _1));
    vm->registerInterrupt(5, boost::bind(&OpSystem::processNew, this, _1));
    vm->registerInterrupt(6, boost::bind(&OpSystem::processFree, this, _1));
    vm->registerInterrupt(7, boost::bind(&OpSystem::processYield, this, _1));

    vm->configureScheduler(50, 0.5, boost::bind(&OpSystem::runScheduler, this, _1));

    vm->registerInterrupt(11, boost::bind(&OpSystem::fsOpen, this, _1));
    vm->registerInterrupt(12, boost::bind(&OpSystem::fsClose, this, _1));
    vm->registerInterrupt(13, boost::bind(&OpSystem::fsSeek, this, _1));
    vm->registerInterrupt(14, boost::bind(&OpSystem::fsTell, this, _1));
    vm->registerInterrupt(15, boost::bind(&OpSystem::fsWrite, this, _1));
    vm->registerInterrupt(16, boost::bind(&OpSystem::fsRead, this, _1));
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

void OS::OpSystem::nvm_loadToFile( const std::string& nvmFname, const std::string fname )
{
    VM::MemoryBlock memory;
    size_t memorySize;
    uint32_t startAddress;
    uint32_t offset;

    VM::ReadFromHex(nvmFname, memory, memorySize, startAddress, offset);

    VM::MemoryBlock newMem(new uint8_t[memorySize + sizeof(startAddress)*2]);
    *((uint32_t*)newMem.get()) = startAddress;
    *(((uint32_t*)newMem.get())+1) = offset;
    memcpy(newMem.get() + sizeof(startAddress)*2, memory.get(), memorySize);
    fileSystem.WriteFile(cwd, fname, (char*)newMem.get(), memorySize + sizeof(startAddress)*2);
}

void OS::OpSystem::nvm_loadTxtFile(const std::string& nvmFname, const std::string fname )
{
	std::ifstream in(nvmFname);
	std::string data;
	std::string temp;
	while(getline(in, temp))
	{
		data+= temp;
		data.append("\n");
	}
	fileSystem.WriteFile(cwd, fname, data.c_str(), data.size());
}

void OS::OpSystem::nvm_load( const std::string& pathfileName,const std::string& name)
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
    newProgram->setState(ProcessState_Ready);
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
        ptr->setState(ProcessState_Ready);
        m_vm->run();
		if(ptr->procstate == ProcessState_Suspended)
		{
			ptr->vm_state = m_vm->getRegisterState();
		}
		else
		{
			ptr->setState(ProcessState_Terminating);
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
    std::for_each(m_processList.begin(), m_processList.end(), boost::bind(&ProcessControlBlock::resetMainClock, _1));
    m_processCount = m_processList.size();
    m_vm->resetRunningTime();
    ptime start(microsec_clock::local_time());
    do {
        m_vm->run();
        m_scheduler->schedule();
    } while (m_vm->isRunning());
    ptime stop(microsec_clock::local_time());

    time_duration len = stop-start;
    double utilization(static_cast<double>(m_vm->getRunningTime().ticks()) / static_cast<double>(len.ticks()));
    double pps(static_cast<double>(m_processCount) / (static_cast<double>(len.total_microseconds()) / 1000000));

    std::cout << std::endl;
    std::cout << "Process Scheduling Statistics:" << std::endl;
    std::cout << "CPU utilization:\t" << utilization*100 << "%" << std::endl;
    std::cout << "Processes / Sec:\t" << pps << std::endl;
    m_scheduler->printStats();
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
	ptr->setState(ProcessState_Suspended);
    ptr->vm_state = temp;
    vm->setRunning(false);
}

void OS::OpSystem::processEnd( VM::VMCore* vm )
{// interrupt 0 / TRP 0
	VM::VMState temp = vm->getRegisterState();
	ProcessControlBlockPtr ptr(getProcess(temp.pid));
    
    // Close all open files
    for (std::map<int, FileHandlePtr>::iterator it(fileTable.begin()); it != fileTable.end(); ++it) {
        fileTable.erase(fileTable.find(it->first));
    }
    ptr->fileTable.clear();

	ptr->setState(ProcessState_Terminating);
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
    proc->setState(ProcessState_Terminating);
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

void OS::OpSystem::formatDisk()
{
    std::cout << "Are you sure you want to erase all data on the disk? (y/N)";
    char r = 'n';
    std::cin >> r;
    if (r == 'Y' || r == 'y') {
        std::cout << "Formatting ..." << std::endl;
    	fileSystem.format();
    }
    cwd = 0;
}

int OS::OpSystem::chdir( const std::string& dir )
{
    cwd = fileSystem.GetDirectoryId(cwd, dir);
    return cwd;
}

std::string OS::OpSystem::pwd()
{
    return fileSystem.GetDirectoryPath(cwd);
}

void OS::OpSystem::mkdir( const std::string& dir )
{
    fileSystem.CreateDirectory(dir, cwd);
}

void OS::OpSystem::ls()
{
    fileSystem.listDirectory(cwd);
}

void OS::OpSystem::cat( std::string fname )
{
    fileSystem.catFile(cwd, fname);
}

void OS::OpSystem::head(std::string fname, int n)
{
	fileSystem.headFile(cwd,fname,n);
}

void OS::OpSystem::tail(std::string fname, int n)
{
	fileSystem.tailFile(cwd,fname,n);
}

void OS::OpSystem::cp(std::string fname, std::string location)
{
	fileSystem.cpFile(cwd, fname, location);
}

void OS::OpSystem::mv(std::string fname, std::string location)
{
	fileSystem.mvFile(cwd, fname, location);
}

void OS::OpSystem::rm(const std::string& name)
{
	fileSystem.rmDirLinFil(cwd, name);
}

void OS::OpSystem::fsOpen( VM::VMCore* vm )
{
	VM::VMState temp = vm->getRegisterState();
	ProcessControlBlockPtr ptr(getProcess(temp.pid));
    char filename[80] = {0};
    VM::MemoryBlock b;
    vm->readMemory(temp.reg[0], b, 80);
    b[79] = 0; // Just in case; make sure we don't read off into nowhere
    strcpy(filename, (const char*)b.get());
    if (!fileOpen(filename)) {
        FileHandlePtr fhptr = boost::make_shared<VirtualFileHandle>(filename, &fileSystem);
        int fh = ++lastFH;
        ptr->fileTable[fh] = fhptr;
        fileTable[fh] = fhptr;
        temp.reg[0] = fh;
    } else {
        temp.reg[0] = 0;
    }
    vm->setRegisterState(temp);
}

void OS::OpSystem::fsClose( VM::VMCore* vm )
{
	VM::VMState temp = vm->getRegisterState();
	ProcessControlBlockPtr ptr(getProcess(temp.pid));
    int fh = temp.reg[0];
    if (ptr->fileTable.find(fh) != ptr->fileTable.end()) {
        ptr->fileTable.erase(ptr->fileTable.find(fh));
        fileTable.erase(fileTable.find(fh));
    }
}

void OS::OpSystem::fsSeek( VM::VMCore* vm )
{
	VM::VMState temp = vm->getRegisterState();
	ProcessControlBlockPtr ptr(getProcess(temp.pid));
    int fh = temp.reg[0];
    int pos = temp.reg[1];
    ptr->fileTable[fh]->seek(pos);
}

void OS::OpSystem::fsTell( VM::VMCore* vm )
{
	VM::VMState temp = vm->getRegisterState();
	ProcessControlBlockPtr ptr(getProcess(temp.pid));
    int fh = temp.reg[0];
    temp.reg[0] = ptr->fileTable[fh]->tell();
    vm->setRegisterState(temp);
}

void OS::OpSystem::fsWrite( VM::VMCore* vm )
{
	VM::VMState temp = vm->getRegisterState();
	ProcessControlBlockPtr ptr(getProcess(temp.pid));
    VM::MemoryBlock b;
    int fh = temp.reg[0];
    vm->readMemory(temp.reg[1], b, temp.reg[2]);
    ptr->fileTable[fh]->write((const char*)b.get(), temp.reg[2]);
    vm->setRegisterState(temp);
}

void OS::OpSystem::fsRead( VM::VMCore* vm )
{
	VM::VMState temp = vm->getRegisterState();
	ProcessControlBlockPtr ptr(getProcess(temp.pid));
    VM::MemoryBlock b(new uint8_t[2048]);
    int fh = temp.reg[0];
    size_t size(temp.reg[2]);
    ptr->fileTable[fh]->read((char*)b.get(), size);
    temp.reg[2] = size;
    vm->writeMemory(temp.reg[1], (const char*)b.get(), temp.reg[2]);
    vm->setRegisterState(temp);
}

bool OS::OpSystem::fileOpen( const std::string& filename )
{
    for (std::map<int, OS::FileHandlePtr>::iterator it = fileTable.begin();
        it != fileTable.end(); ++it) {
        if (it->second->getFilename() == filename) {
            return true;
        }
    }
    return false;
}

