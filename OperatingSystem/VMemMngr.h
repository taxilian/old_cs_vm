#pragma once
#ifndef H_VMEMMANAGEMENT
#define H_VMEMMANAGEMENT
#include "VMCore.h"
#include "FileSystem.h"
#include <boost\lexical_cast.hpp>
namespace OS {
class VMemMngr
{
private:
	VM::VMCore* vm_core;
	FileSystem* fSystem;
	int pageSize;
	int swapDirectory;
	std::list<VM::PTEntry>::iterator iter;
public:
	VMemMngr(VM::VMCore* vm, FileSystem* _fileSystem);
	void swapIn();
	void swapOut();
	void getPage();
	//preconditions: ptEntry contains information about page being save to virtual memory.
	//memory contains a block of size 512.
	//postconditions: a page is save into virtual memory. 
	void saveToVMem(VM::PTEntry* ptEntry,  VM::MemoryBlock& memory);
};
}
#endif