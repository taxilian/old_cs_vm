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
        int nextFrame;
    public:
        VMemMngr(VM::VMCore* vm, FileSystem* _fileSystem);
        void swapIn(int);
        void swapOut(int);
        void getPage(int);
    };
}
#endif