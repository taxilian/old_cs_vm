#include <boost\date_time\posix_time\posix_time.hpp>
#include "FileSystem.h"

using namespace OS;
using namespace std;

OS::FileSystem::FileSystem( VM::VirtualDisk* disk ) : disk(disk), iNodeStartBlock(1)
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
    return getStructData<Directory>(block, 0);
}

void OS::FileSystem::saveDirectory( const uint32_t block, const OS::Directory& dir)
{
    writeStructData(block, 0, dir);
}
uint32_t OS::FileSystem::findFreeBlock()
{
    unsigned char data[blkSize];
	for(int i = 1; i < VM::VirtualDisk::DEFAULT_DISK_SIZE; i++)
	{
        disk->ReadFromDisk(i, 1, reinterpret_cast<char*>(data));
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
    // Create inodes -- conveniently, they are really just 0 when they are empty
    iNFile emptyINode;
    char data[blkSize];
    memset(data, 0xFE, blkSize);
    memset(data, 0, sizeof(iNFile) * iNodesPerBlock);
    int blkCount = iNodeCount / iNodesPerBlock;
    int blk = 0;
    for (int i = 0; i < blkCount; i++) {
        int blk = findFreeBlock();
        if (i == 0) assert(blk == iNodeStartBlock);
        disk->WriteToDisk(blk, 1, data);
    }
}

int OS::FileSystem::GetDirectoryId( int cwd, const std::string& dirName )
{
    Directory dir = getDirectory(cwd);
    for (int i = 0; i < maxEntriesPerBlock; i++) {
        if (dir.entries[i].type == TYPE_directory
            && dir.entries[i].name == dirName) {
            return dir.entries[i].ptr;
        }
    }
    // If we got this far, there were no empty blocks in that directory entry and
    // we need to chain
    if (dir.next) {
        // If there is another directory block after this one then chain to it
        return GetDirectoryId(dir.next, dirName);
    } else {
        return -1; // invalid directory
    }
}

Entry OS::FileSystem::getDirectoryEntry( int parent, int blockNumber )
{
    Directory dir = getDirectory(parent);
    for (int i = 0; i < maxEntriesPerBlock; i++) {
        if (dir.entries[i].type == TYPE_directory
            && dir.entries[i].ptr == blockNumber) {
            return dir.entries[i];
        }
    }
    if (dir.next) {
        return getDirectoryEntry(dir.next, blockNumber);
    } else {
        throw std::runtime_error("Invalid directory");
    }
}

OS::Entry OS::FileSystem::getDirectoryEntry( int cwd, const std::string& fileName )
{
    static Entry nullEntry = {TYPE_empty, {0}, 0};
    Directory dir = getDirectory(cwd);
    for (int i = 0; i < maxEntriesPerBlock; i++) {
        if (dir.entries[i].name == fileName) {
            return dir.entries[i];
        }
    }
    if (dir.next) {
        return getDirectoryEntry(dir.next, fileName);
    } else {
        return nullEntry;
    }
}

std::string OS::FileSystem::GetDirectoryPath( int cwd )
{
    if (cwd == 0) {
        return "";
    }
    Directory dir = getDirectory(cwd);
    
    std::stringstream ss;
    ss << GetDirectoryPath( dir.entries[1].ptr ) << "/";
    ss << getDirectoryEntry( dir.entries[1].ptr, cwd).name;
    return ss.str();
}

void OS::FileSystem::listDirectory( int cwd )
{
    Directory dir = getDirectory(cwd);
    for (int i = 0; i < maxEntriesPerBlock; i++) {
        switch(dir.entries[i].type) {
        case TYPE_directory:
            cout << dir.entries[i].name << "/" << endl;
            break;
        case TYPE_file:
            cout << dir.entries[i].name << endl;
            break;
        default:
            break;
        }
    }
    if (dir.next) {
        listDirectory(dir.next);
    }
}

OS::iNFile OS::FileSystem::getFileNode( const uint32_t iNodeNum )
{
    boost::tuple<int, int> nodeLoc = getINodeBlockAndOffset(iNodeNum);
    return getStructData<iNFile>(boost::get<0>(nodeLoc)+iNodeStartBlock, boost::get<1>(nodeLoc) * sizeof(iNFile));
}
void OS::FileSystem::saveFileNode( const uint32_t iNodeNum, const iNFile& node )
{
    boost::tuple<int, int> nodeLoc = getINodeBlockAndOffset(iNodeNum);
    writeStructData(boost::get<0>(nodeLoc)+iNodeStartBlock, boost::get<1>(nodeLoc) * sizeof(node), node);
}

OS::iNLink OS::FileSystem::getLinkNode( const uint32_t iNodeNum )
{
    boost::tuple<int, int> nodeLoc = getINodeBlockAndOffset(iNodeNum);
    return getStructData<iNLink>(boost::get<0>(nodeLoc), boost::get<1>(nodeLoc) * sizeof(iNLink));
}
void OS::FileSystem::saveLinkNode( const uint32_t iNodeNum, const iNLink& node )
{
    boost::tuple<int, int> nodeLoc = getINodeBlockAndOffset(iNodeNum);
    writeStructData(boost::get<0>(nodeLoc), boost::get<1>(nodeLoc) * sizeof(node), node);
}

boost::tuple<int, int> OS::FileSystem::getINodeBlockAndOffset( int nodeNum )
{
    int block = nodeNum / iNodesPerBlock;
    int num = nodeNum % iNodesPerBlock;
    return boost::make_tuple(block, num);
}

int OS::FileSystem::findFreeINode()
{
    // Loop through iNode blocks
    for (int i = 0; i < iNodeCount / iNodesPerBlock; i++) {
        char data[blkSize];
        disk->ReadFromDisk(iNodeStartBlock + i, 1, data);
        // Loop through iNodes in the blocks
        for (int n = 0; n < iNodesPerBlock; n++) {
            if (data[sizeof(iNFile)*n] == 0) { // if the refcount is 0, unused
                return n + (iNodesPerBlock * i);
            }
        }
    }
    throw std::runtime_error("No free inodes left!");
}

uint64_t getCurrentTime()
{
    using namespace boost::posix_time;
    ptime t(boost::posix_time::second_clock::local_time());
    ptime epoch(boost::gregorian::date(1970,1,1));
    time_duration::sec_type x = (t - epoch).total_seconds();
    return x;
}

bool OS::FileSystem::WriteFile( int cwd, const std::string& file, char* data, size_t size )
{
    static char gData[blkSize] = {0}; // We'll use this for partial blocks
    // TODO: Find out if this file already exists in the directory; if so, overwrite it
    int iNodeIdx = findFreeINode();
    Entry fileEntry;
    fileEntry.ptr = iNodeIdx;
    strcpy(fileEntry.name, file.c_str());
    fileEntry.type = TYPE_file;
    iNFile inode;
    memset(&inode, 0, sizeof(inode));
    inode.creationDate = getCurrentTime();
    inode.modifyDate = getCurrentTime();
    inode.fileSize = size;
    inode.refCount = 1;

    // Calculate how many blocks we'll need
    int blocksNeeded = size / blkSize + (size % blkSize == 0 ? 0 : 1);
    assert(blocksNeeded <= fileDataBlks);
    int curBlock = 0;
    while (curBlock < blocksNeeded) {
        int blkNum = findFreeBlock();
        inode.dataBLKS[curBlock] = blkNum;
        if (size - (curBlock*blkSize) < blkSize) {
            // It's only a "partial" block, so we'll have to pad it
            memcpy(gData, data+(curBlock*blkSize), size - (curBlock*blkSize));
            disk->WriteToDisk(blkNum, 1, gData);
        } else {
            disk->WriteToDisk(blkNum, 1, data+(curBlock*blkSize));
        }
        ++curBlock;
    }
    saveFileNode(iNodeIdx, inode);
    addDirectoryEntry(cwd, fileEntry);
    return true;
}

void OS::FileSystem::catFile( int cwd, const std::string& file )
{
    static char buffer[blkSize];
    Entry entry = getDirectoryEntry(cwd, file);
    if (entry.type == TYPE_empty) {
        cout << "No such file" << endl;
        return;
    } else if (entry.type != TYPE_file) {
        cout << "Unsupported request" << endl;
    }
    iNFile fileNode = getFileNode(entry.ptr);
    int32_t dataLeft = fileNode.fileSize;
    std::stringstream ss;
    for (int i = 0; i < fileDataBlks && dataLeft > 0; ++i) {
        if (fileNode.dataBLKS[i]) {
            disk->ReadFromDisk(fileNode.dataBLKS[i], 1, buffer);
            ss << std::hex;
            ss.write(buffer, (dataLeft > blkSize ? blkSize : dataLeft));
            dataLeft -= blkSize;
        }
    }
    cout << ss.str() << endl << "END OF FILE" << endl;
}

