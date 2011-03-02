#pragma once
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <string>

#include "VMCore.h"
#include "VirtualDisk.h"

namespace OS {

    const size_t fileDataBlks = 20;//every block is 512 bytes which gives a maximum of 10KB file size.
    const size_t lengthPath = 20;
    const size_t linkPath = 80;
    const size_t blkSize = VM::VirtualDisk::BLOCK_SIZE;
    const size_t numOfEntries = 10;//maximum number of directory or file entries in a directory.


    enum iNType{TYPE_empty = 0, TYPE_directory,TYPE_file,TYPE_link};
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
    const size_t maxEntriesPerBlock = (blkSize - (/*sizeof(size_t) + */sizeof(uint32_t))) / sizeof(Entry);
    struct Directory{
        //size_t size; //Got looking at it and decided we don't actually need this
        uint32_t next;
        Entry entries[maxEntriesPerBlock];//if entries exceed 10 then Inode points to next dir block.
    };

    class FileSystem
    {
    public:
        FileSystem(VM::VirtualDisk* disk);
        ~FileSystem();

    public:
        void CreateDirectory(const std::string name, const uint32_t parent);
        std::string GetDirectoryPath( int cwd );
		void format();
        int GetDirectoryId( int cwd, const std::string& dir );
        void listDirectory( int cwd );
    protected:
        Directory getDirectory(const uint32_t block);
        void saveDirectory( const uint32_t block, const OS::Directory& dir);
        uint32_t findFreeBlock();
        void freeBlock(uint32_t num);
        void addDirectoryEntry( const uint32_t dirBlock, const Entry& entry );
        Entry getDirectoryEntry( int parent, int blockNumber );
    private:
        VM::VirtualDisk* disk;
    };

}
#endif