#pragma once

namespace OS {
    class FileHandle
    {
    public:
        virtual ~FileHandle(void) {};

    public:
        virtual void seek(int offset) = 0;
        virtual int tell() = 0;
        virtual void write(int offset, const char* bytes, size_t len) = 0;
        virtual void read(int offset, const char* bytes, size_t& len) = 0;
    };
};
