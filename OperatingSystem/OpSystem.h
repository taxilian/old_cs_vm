#pragma once
#ifndef OpSystem_h__
#define OpSystem_h__

#include <string>

#include "VMCore.h"

namespace OS {
    class OpSystem
    {
    public:
        OpSystem(VM::VMCore* vm);
        ~OpSystem(void);

        void load(const std::string& fileName);

	private:
        VM::VMCore* m_vm;
    };
}
#endif // OpSystem_h__
