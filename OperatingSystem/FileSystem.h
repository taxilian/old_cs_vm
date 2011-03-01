#pragma once
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <string>

#include "VMCore.h"
#include "VirtualDisk.h"

namespace OS {

    const size_t fileDataBlks = 20;//every block is 512 bytes which gives a maximum of 10KB file size.
    const size_t lengthPath = 80;// 20 * 4 is 80 bytes, our paths should also be 80 bytes
    const size_t blkSize = VM::VirtualDisk::BLOCK_SIZE;
    const size_t numOfEntries = 10;//maximum number of directory or file entries in a directory.


    enum iNType{TYPE_directory,TYPE_file,TYPE_link};
    struct iNFile {
        uint64_t creationDate;//YYYYMMDD
        uint64_t modifyDate;//YYYYMMDD
        uint32_t fileSize;
        uint32_t dataBLKS[fileDataBlks];
    };
    struct iNLink {
        uint64_t creationDate;//YYYYMMDD
        uint64_t modifyDate;//YYYYMMDD
        uint32_t fileSize;
        char pathName[lengthPath];
    };
    struct Data{
        char rawData[blkSize]; 
        Data(){
            memset(rawData, 0xF0, blkSize);
        }
    };
    struct Entry{
        iNType type;
        char name[lengthPath];
        uint32_t ptr;
    };
    const size_t maxEntriesPerBlock = (blkSize - (sizeof(size_t) + sizeof(uint32_t))) / sizeof(Entry);
    struct Directory{
        size_t size;//number of entries in directory.
        uint32_t next;
        Entry entries[maxEntriesPerBlock];//if entries exceed 10 then Inode points to next dir block.
    };

    class FileSystem
    {
    public:
        FileSystem(VM::VirtualDisk* disk);
        ~FileSystem();

    public:
        Directory getDirectory(const uint32_t block);
        void CreateDirectory(const std::string name, const uint32_t parent);
        void SaveDirectory( const uint32_t block, const OS::Directory& dir);

    protected:
        uint32_t findFreeBlock();
        void freeBlock(uint32_t num);

    private:
        VM::VirtualDisk* disk;
    };

}
#endif