#pragma once
#include <string>
#include "FileSystem.h"
#include "FileHandle.h"

namespace OS {
    class VirtualFileHandle :
        public OS::FileHandle
    {
    public:
        VirtualFileHandle(const std::string& filename, FileSystem* fs);
        ~VirtualFileHandle(void);

        virtual void seek(int offset) = 0;
        virtual int tell() = 0;
        virtual void write(const char* bytes, size_t len) = 0;
        virtual void read(char* bytes, size_t& len) = 0;

    private:
        const std::string m_filename;
        FileSystem* fs;
        size_t pos;
    };
};