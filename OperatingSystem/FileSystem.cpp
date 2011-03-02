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
    memset(&nDir, 0, sizeof(nDir)); // Init the struct to 0s
    uint32_t nDirBlock;

    if (name.empty() && parent == 0) {
        nDirBlock = 0;
        saveDirectory(0, nDir);
    } else {
        nDirBlock = this->findFreeBlock();
        saveDirectory(nDirBlock, nDir);
    }
    // Set up the new directory
    Entry newEntry;
    
    // Add entry for "."
    strcpy(newEntry.name, ".");
    newEntry.type = TYPE_directory;
    newEntry.ptr = nDirBlock;
    addDirectoryEntry(nDirBlock, newEntry);
    // Add entry for ".."
    strcpy(newEntry.name, "..");
    newEntry.ptr = parent;
    addDirectoryEntry(nDirBlock, newEntry);

    if (nDirBlock != parent) {
        // Add a pointer to the new directory to the parent dir
        Entry entry;
        strcpy(entry.name, name.c_str());
        entry.type = TYPE_directory;
        entry.ptr = nDirBlock;
        addDirectoryEntry(parent, entry);
    }
}

void OS::FileSystem::addDirectoryEntry( const uint32_t dirBlock, const Entry& entry )
{
    Directory dir = getDirectory(dirBlock);
    for (int i = 0; i < maxEntriesPerBlock; i++) {
        if (dir.entries[i].type == TYPE_empty) {
            dir.entries[i] = entry;
            saveDirectory(dirBlock, dir);
            return;
        }
    }
    // If we got this far, there were no empty blocks in that directory entry and
    // we need to chain
    if (dir.next) {
        // If there is another directory block after this one then chain to it
        addDirectoryEntry(dir.next, entry);
    } else {
        // If there is not, it is time to create one
        dir.next = findFreeBlock();
        Directory dirNext;
        dirNext.next = 0;
        dir.entries[0] = entry;
        saveDirectory(dir.next, dirNext); // Save new entry
        saveDirectory(dirBlock, dir); // Save previous entry with new chain
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

void OS::FileSystem::saveDirectory( const uint32_t block, const OS::Directory& dir)
{
    char data[blkSize];
    memcpy(data, &dir, sizeof(dir));
    disk->WriteToDisk(block, 1, data);
}
uint32_t OS::FileSystem::findFreeBlock()
{
    char data[blkSize];
	for(int i = 1; i < VM::VirtualDisk::DEFAULT_DISK_SIZE; i++)
	{
        disk->ReadFromDisk(i, 1, data);
        if (data[0] == 0xF0
            && data[1] == 0xF2
            && data[2] == 0xF1
            && data[3] == 0xF3) {
            return i;
        }
	}
    throw std::runtime_error("Out of disk space");
}
void OS::FileSystem::freeBlock(uint32_t num)
{
	static char init[VM::VirtualDisk::BLOCK_SIZE];
    static bool set(false);
    if (!set) {
        init[0] = 0xF0;
        init[1] = 0xF2;
        init[2] = 0xF1;
        init[3] = 0xF3;
        set = true;
    }
    disk->WriteToDisk(num, 1, init);
}
void OS::FileSystem::format()
{
	disk->Erase();
	for(int i = 0; i < VM::VirtualDisk::DEFAULT_DISK_SIZE; i++)
	{
		freeBlock(i);
	}
    // Create root directory
    CreateDirectory("", 0);
}
