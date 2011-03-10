#pragma once

namespace OS {
    class FileHandle
    {
    public:
        virtual ~FileHandle(void) {};

    public:
        virtual void seek(int offset) = 0;
        virtual int tell() = 0;
        virtual void write(const char* bytes, size_t len) = 0;
        virtual void read(char* bytes, size_t& len) = 0;
    };
    typedef boost::shared_ptr<FileHandle> FileHandlePtr;
};
