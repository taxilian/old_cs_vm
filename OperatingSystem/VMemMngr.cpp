#include "VMemMngr.h"
using namespace OS;
OS::VMemMngr::VMemMngr(VM::VMCore* vm, FileSystem* _fileSystem):vm_core(vm), fSystem(_fileSystem)
{
	pageSize = VM::VMCore::FRAME_SIZE;
	swapDirectory= 0;
    nextFrame = 0;
}
void VMemMngr::swapIn(int pageNeeded)
{
	VM::MemoryBlock memory;
	size_t memorySize = 0;
	std::string pageFile = boost::lexical_cast<std::string>(pageNeeded);
    try {
    	fSystem->readFile(swapDirectory,pageFile,memory,memorySize);
    	fSystem->rmDirLinFil(swapDirectory, pageFile);//remove page from virtual memory.
    } catch (...) {
        // If the frame doesn't exist, just use garbage data.
        memory = VM::MemoryBlock(new uint8_t[pageSize]);
    }
    vm_core->writeFrame(nextFrame, pageNeeded, (const char*)memory.get());
}
void VMemMngr::swapOut(int page)
{
    if (page == -1) {
        return; // there isn't an active frame to swap out
    }
	int address = 0;
	std::string pageFile = boost::lexical_cast<std::string>(page);
	VM::MemoryBlock memory;
	vm_core->readFrame(nextFrame, memory);
	fSystem->WriteFile(swapDirectory,pageFile,(char*)memory.get(),pageSize);
}
void VMemMngr::getPage(int page)
{
    std::cout << "Page fault! Swapping " << vm_core->whichPage(nextFrame) << " out of frame ";
    std::cout << nextFrame << " and replacing it with page " << page << std::endl;
	swapDirectory = fSystem->GetDirectoryId(0,"SwapFile");
	swapOut(vm_core->whichPage(nextFrame));
	swapIn(page);
    if (++nextFrame >= VM::VMCore::FRAME_COUNT)
        nextFrame = 0;
}