#pragma once
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <string>
#include <boost/tuple/tuple.hpp>
#include <boost/static_assert.hpp>

#include "VMCore.h"
#include "VirtualDisk.h"
#include <stdexcept>

namespace OS {

    class FileSystemError : public std::runtime_error {
    public:
        FileSystemError(const std::string& msg) : std::runtime_error(msg.c_str()) {}
    };
    class FileNotFoundError : public FileSystemError {
    public:
        FileNotFoundError(const std::string& msg) : FileSystemError(msg) {}
    };

    const size_t fileDataBlks = 70;//every block is 512 bytes which gives a maximum of 35KB file size.
    const size_t lengthPath = 20;
    const size_t linkPath = 280;
    const size_t blkSize = VM::VirtualDisk::BLOCK_SIZE;

    enum iNType{TYPE_empty = 0, TYPE_directory,TYPE_file,TYPE_link};
    struct iNFile {
        uint8_t refCount;
        uint64_t creationDate;
        uint64_t modifyDate;
        uint32_t fileSize;
        uint32_t dataBLKS[fileDataBlks];
    };
    struct iNLink {
        uint8_t refCount;
        uint64_t creationDate;
        uint64_t modifyDate;
        uint32_t fileSize;
        char pathName[linkPath];
    };

    // Make sure that the iNode types are the same size!
    BOOST_STATIC_ASSERT(sizeof(iNFile) == sizeof(iNLink));

    const size_t iNodesPerBlock = blkSize / sizeof(iNFile);
    const size_t iNodeCount = ((256 / iNodesPerBlock) + 1) * iNodesPerBlock; // Maximum number of files supported on the disk

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
        bool WriteFile( int cwd, const std::string& file, const char* data, size_t size );
        void listDirectory( int cwd );
        void catFile( int cwd, const std::string& file );
		void headFile(int cwd, const std::string& file, int n);
		void tailFile(int cwd, const std::string& file, int n);
        Entry getFileEntry( int cwd, const std::string& file );

        void cpFile(int cwd, const std::string& file, const std::string& dest);
		void mvFile(int cwd, const std::string& file, const std::string& dest);
		void rmDirLinFil(int _cwd, const std::string& name);
		void dfFile();
		void duFile();
		void touchFile(int cwd, std::string& file, std::string& date);
		void lnFile(int cwd, std::string& file, std::string& lname, std::string& sym);

        void moveFile( int cwd, const std::string& src, const std::string& dest );
        void readFile( int cwd, const std::string& filename, VM::MemoryBlock& block, size_t& size );
        bool fileExists( int cwd, const std::string filename );
        size_t getFileSize( int cwd, const std::string& filename );

    protected:
        Directory getDirectory(const uint32_t block);
        void saveDirectory( const uint32_t block, const OS::Directory& dir);
        iNFile getFileNode(const uint32_t iNodeNum);
        void saveFileNode( const uint32_t iNodeNum, const iNFile& node);
        iNLink getLinkNode(const uint32_t iNodeNum);
        void saveLinkNode( const uint32_t iNodeNum, const iNLink& node);
        int findFreeINode();
        uint32_t findFreeBlock();
		int totalFreeBlocks();
        void freeBlock(uint32_t num);
        void addDirectoryEntry( const uint32_t dirBlock, const Entry& entry );
        Entry getDirectoryEntry( int parent, int blockNumber );
        Entry getDirectoryEntry( int parent, const std::string& fileName );
		void clearDirectoryEntry(int parent, const std::string& fileName );
        boost::tuple<int, int> getINodeBlockAndOffset(int nodeNum);
        void readFileContents( Entry &entry, VM::MemoryBlock& mem, size_t& size );
        boost::tuple<int, std::string> resolvePath(int cwd, const std::string& fileName);
		std::string timeToString(uint64_t& t);

        template <class T>
        T getStructData(int block, int offset)
        {
            T dt;
            char data[blkSize];
            disk->ReadFromDisk(block, 1, data);
            memcpy(&dt, data+offset, sizeof(dt));
            return dt;
        }

        template <class T>
        void writeStructData(int block, int offset, const T& st)
        {
            char data[blkSize];
            disk->ReadFromDisk(block, 1, data);
            memcpy(data+offset, &st, sizeof(st));
            disk->WriteToDisk(block, 1, data);
        }
    private:
        VM::VirtualDisk* disk;
        int iNodeStartBlock;
    };

}
#endif