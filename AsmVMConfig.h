/**
 * AsmVMConfig.h
 *
 * Richard Bateman
 * Virtual Machine Assembly Configuration object
 */

#pragma once
#ifndef H_ASMVMCONFIG
#define H_ASMVMCONFIG

#include <string>
#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "VMConfig.h"

class AsmVMConfig : public VMConfig
{
protected:
public:
    AsmVMConfig(void);
    virtual ~AsmVMConfig(void);
};

#endif

