#include "VMemMngr.h"
using namespace OS;
OS::VMemMngr::VMemMngr(VM::VMCore* vm, FileSystem* _fileSystem):vm_core(vm), fSystem(_fileSystem)
{
	pageSize = 512;
	swapDirectory= 0;
	iter = vm_core->framesInfo.begin();
}
void VMemMngr::swapIn()
{
	VM::MemoryBlock memory;
	std::string pageFile;
	size_t memorySize = 0;
	char processID[32];
	char pageNumber[32];
	_itoa_s(vm_core->pageNeeded.pid,processID,32,10);
	_itoa_s(vm_core->pageNeeded.pageNum,pageNumber,32,10);
	pageFile = processID;
	pageFile += "_";
	pageFile += pageNumber;
	pageFile += ".hexe";
	fSystem->readFile(swapDirectory,pageFile,memory,memorySize);
	iter->pageNum = vm_core->pageNeeded.pageNum;
	iter->pid = vm_core->pageNeeded.pid;
	iter->valid = true;
	for(std::list<VM::PTEntry>::iterator i = vm_core->PageTable.begin(); i != vm_core->PageTable.end(); i++)
	{
		if((iter->pid == i->pid) && (iter->pageNum == i->pageNum))
			i->valid = true;
	}
	vm_core->writeFrame(vm_core->currentFrame*pageSize,(const char*)memory.get(),pageSize);
}
void VMemMngr::swapOut()
{
	int address = 0;
	std::string pageFile;
	if(vm_core->currentFrame == 7)
	{
		address = 0;
		vm_core->currentFrame = 0;
		iter = vm_core->framesInfo.begin();
	}
	else
	{
		vm_core->currentFrame++;
		address = vm_core->currentFrame * pageSize;
		iter++;
	}
	char processID[32];
	char pageNumber[32];
	_itoa_s(iter->pid,processID,32,10);
	_itoa_s(iter->pageNum,pageNumber,32,10);
	pageFile = processID;
	pageFile += "_";
	pageFile += pageNumber;
	pageFile += ".hexe";
	VM::MemoryBlock memory;
	vm_core->readFrame(address,memory,pageSize);
	fSystem->WriteFile(swapDirectory,pageFile,(char*)memory.get(),pageSize);
	for(std::list<VM::PTEntry>::iterator i = vm_core->PageTable.begin(); i != vm_core->PageTable.end(); i++)
	{
		if((iter->pid == i->pid) && (iter->pageNum == i->pageNum))
			i->valid = false;
	}
}
void VMemMngr::getPage()
{
	swapDirectory= fSystem->GetDirectoryId(0,"SwapFile");
	swapOut();
	swapIn();
}