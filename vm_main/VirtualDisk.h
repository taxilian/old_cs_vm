#pragma once

#include <string>
#include <iostream>
#include <fstream>

namespace VM {

    class VirtualDisk
    {
    public:
        const static int BLOCK_SIZE = 512;
        const static int DEFAULT_DISK_SIZE = 2048;
    public:
        VirtualDisk(const std::string& fileName);
        ~VirtualDisk(void);

        void ReadFromDisk(int block, size_t len, char* retPtr);
        void WriteToDisk(int block, size_t len, const char* data);
        void Erase(int blockCount = DEFAULT_DISK_SIZE);

    private:
        std::string m_filename;

    };
};