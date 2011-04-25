#pragma once
#ifndef H_VMEMMANAGEMENT
#define H_VMEMMANAGEMENT
#include "VMCore.h"
#include "FileSystem.h"
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
};
}
#endif