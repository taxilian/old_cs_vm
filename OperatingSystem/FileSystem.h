#pragma once
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "VMCore.h"
#include "VirtualDisk.h"

namespace OS {

    const size_t fileDataBlks = 20;//every block is 512 bytes which gives a maximum of 10KB file size.
    const size_t lengthPath = 80;// 20 * 4 is 80 bytes, our paths should also be 80 bytes
    const size_t blkSize = VM::VirtualDisk::BLOCK_SIZE;
    const size_t numOfEntries = 10;//maximum number of directory or file entries in a directory.

    enum iNType{directory,file,link};
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
        uint32_t iNode;
    };
    struct Directory{
        size_t size;//number of entries in directory.
        uint32_t next;
        Entry entries[numOfEntries];//if entries exceed 10 then Inode points to next dir block.
    };
    class FileSystem
    {

    };

}
#endif