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
public:
	VMemMngr(VM::VMCore* vm, FileSystem* _fileSystem);
	void swapIn();
	void swapOut();
};
}
#endif