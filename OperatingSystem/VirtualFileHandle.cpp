#include "VirtualFileHandle.h"
#include "FileSystem.h"

using namespace OS;

VirtualFileHandle::VirtualFileHandle(const std::string& filename, FileSystem* fs) : m_filename(filename), fs(fs), pos(0)
{
    if (filename[0] != '/') {
        throw FileSystemError("Invalid filename");
    }
}


VirtualFileHandle::~VirtualFileHandle(void)
{
}

void OS::VirtualFileHandle::seek( int offset )
{
    pos = offset;
}

int OS::VirtualFileHandle::tell()
{
    return pos;
}

void OS::VirtualFileHandle::write( const char* bytes, size_t len )
{
    size_t size;
    VM::MemoryBlock oldblock;
    VM::MemoryBlock newblock;
    size_t newsize;
    fs->readFile(0, m_filename, oldblock, size);
    if (pos >= size) {
        pos = size;
    }
    if (pos+len > size) {
        newblock = VM::MemoryBlock(new uint8_t[size+len]);
        memcpy(newblock.get(), oldblock.get(), size);
        newsize = len + size;
    }
    memcpy(newblock.get() + pos, bytes, len);
    fs->WriteFile(0, m_filename, bytes, newsize);
}

void OS::VirtualFileHandle::read( char* bytes, size_t& len )
{
    size_t size;
    VM::MemoryBlock block;
    fs->readFile(0, m_filename, block, size);
    if (pos >= size) {
        len = 0;
        return;
    } else if (pos + len > size) {
        len = size - pos;
        memcpy(bytes, block.get() + pos, len);
    } else {
        memcpy(bytes, block.get() + pos, len);
    }
}

std::string OS::VirtualFileHandle::getFilename()
{
    return m_filename;
}
