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
	size_t memorySize = 0;
	std::string pageFile = boost::lexical_cast<std::string>(vm_core->pageNeeded.pageNum);
	fSystem->readFile(swapDirectory,pageFile,memory,memorySize);
	iter->pageNum = vm_core->pageNeeded.pageNum;
	iter->valid = true;
	for(std::list<VM::PTEntry>::iterator i = vm_core->PageTable.begin(); i != vm_core->PageTable.end(); i++)
	{
		if(iter->pageNum == i->pageNum)
		{
			i->valid = true;
			break;
		}
	}
	vm_core->writeFrame(vm_core->currentFrame*pageSize,(const char*)memory.get(),pageSize);
	fSystem->rmDirLinFil(swapDirectory,pageFile);//remove page from virtual memory.
}
void VMemMngr::swapOut()
{
	int address = 0;
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
	std::string pageFile =boost::lexical_cast<std::string>(iter->pageNum);
	VM::MemoryBlock memory;
	vm_core->readFrame(address,memory,pageSize);
	fSystem->WriteFile(swapDirectory,pageFile,(char*)memory.get(),pageSize);
	for(std::list<VM::PTEntry>::iterator i = vm_core->PageTable.begin(); i != vm_core->PageTable.end(); i++)
	{
		if(iter->pageNum == i->pageNum)
		{
			i->valid = false;
			break;
		}
	}
}
void VMemMngr::getPage()
{
	swapDirectory= fSystem->GetDirectoryId(0,"SwapFile");
	swapOut();
	swapIn();
}
void VMemMngr::saveToVMem(VM::PTEntry* ptEntry,  VM::MemoryBlock& memory)
{
	assert(ptEntry->pageNum >= 0);
	std::string pageFile = boost::lexical_cast<std::string>(ptEntry->pageNum);
	fSystem->WriteFile(swapDirectory,pageFile,(char*)memory.get(),pageSize);
	vm_core->PageTable.push_back(*ptEntry);
}