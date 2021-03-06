#include "VirtualDisk.h"

using namespace std;

VM::VirtualDisk::VirtualDisk(const std::string& fileName) : m_filename(fileName)
{
}

VM::VirtualDisk::~VirtualDisk(void)
{
}

void VM::VirtualDisk::ReadFromDisk( int block, size_t len, char* retPtr )
{
    ifstream file(m_filename.c_str(), ios::in|ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file!");
    }
    file.seekg(BLOCK_SIZE * block);
    file.read(retPtr, BLOCK_SIZE);
    file.close();
}

void VM::VirtualDisk::WriteToDisk( int block, size_t len, const char* data )
{
    fstream file(m_filename.c_str(), ios::in|ios::out|ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file!");
    }
    file.seekp(BLOCK_SIZE * block, ios::beg);
    file.write(data, BLOCK_SIZE);
    file.close();
}

void VM::VirtualDisk::Erase(int blockCount)
{
    char empty[BLOCK_SIZE] = {0};
    memset(empty, 0, BLOCK_SIZE);

    ofstream file(m_filename.c_str(), ios::out|ios::binary|ios::trunc);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file!");
    }
    
    for (int i = 0; i < blockCount; i++) {
        file.write(empty, BLOCK_SIZE);
    }
    file.close();
}