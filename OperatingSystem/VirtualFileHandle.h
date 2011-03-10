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

        virtual void seek(int offset);
        virtual int tell();
        virtual void write(const char* bytes, size_t len);
        virtual void read(char* bytes, size_t& len);
        virtual std::string getFilename();

    private:
        const std::string m_filename;
        FileSystem* fs;
        size_t pos;
    };
    typedef boost::shared_ptr<VirtualFileHandle> VFHandlerPtr;
};
