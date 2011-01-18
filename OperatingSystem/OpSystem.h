#pragma once
#ifndef OpSystem_h__
#define OpSystem_h__

#include <string>

#include "VMCore.h"

namespace OS {
    class OpSystem
    {
    public:
        OpSystem(void);
        ~OpSystem(void);

        void load(const std::string& fileName);
    };
}
#endif // OpSystem_h__
