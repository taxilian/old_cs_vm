#include "FileSystem.h"

using namespace OS;

OS::FileSystem::FileSystem( VM::VirtualDisk* disk ) : disk(disk)
{

}

OS::FileSystem::~FileSystem()
{

}

void OS::FileSystem::CreateDirectory( const std::string name, const uint32_t parent )
{
    assert(name.size() < lengthPath);
    OS::Directory pDir = getDirectory(parent);
    OS::Directory nDir;
    uint32_t nDirBlock;

    // Set up the new directory
    nDir.size = 2;
    strcpy(nDir.entries[0].name, ".");
    strcpy(nDir.entries[1].name, "..");
    nDir.entries[0].type = TYPE_directory;
    nDir.entries[1].type = TYPE_directory;
    nDir.entries[0].ptr = parent; // Block 0, or this one
    nDir.entries[1].ptr = parent;

    if (name.empty() && parent == 0) {
        SaveDirectory(0, nDir);
    } else {
        nDirBlock = this->findFreeBlock();
        SaveDirectory(nDirBlock, nDir);
        int entry = pDir.size++;
        pDir.entries[entry].ptr = nDirBlock;
        strcpy(pDir.entries[entry].name, name.c_str());
        SaveDirectory(parent, pDir);
    }
}

OS::Directory OS::FileSystem::getDirectory( const uint32_t block )
{
    OS::Directory dir;
    char data[blkSize];
    disk->ReadFromDisk(block, 1, data);
    memcpy(&dir, data, sizeof(dir));
    return dir;
}

void OS::FileSystem::SaveDirectory( const uint32_t block, const OS::Directory& dir)
{
    char data[blkSize];
    memcpy(data, &dir, sizeof(dir));
    disk->WriteToDisk(block, 1, data);
}