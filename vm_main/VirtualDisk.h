#pragma once

#include <string>
#include <iostream>
#include <fstream>

class VirtualDisk
{
public:
    const static int BLOCK_SIZE = 512;
public:
    VirtualDisk(const std::string& fileName);
    ~VirtualDisk(void);

    void ReadFromDisk(int block, size_t len, char* retPtr);
    void WriteToDisk(int block, size_t len, char* data);
    void Erase(int blockCount = 2048);

private:
    std::string m_filename;

};
