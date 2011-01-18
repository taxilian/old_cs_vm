#include "HexWriter.h"
#include <iostream>

#include "OpSystem.h"

using namespace OS;

OpSystem::OpSystem(void)
{
}


OpSystem::~OpSystem(void)
{
}

void OS::OpSystem::load( const std::string& fileName )
{
    VM::MemoryBlock memory;
    size_t memorySize;
    uint32_t startAddress;
    uint32_t offset;
    VM::ReadFromHex(fileName, memory, memorySize, startAddress, offset);

    std::cout << "Loaded " << fileName << std::endl;
}
