/**
 * AsmVMConfig.h
 *
 * Richard Bateman
 * Virtual Machine Assembly Configuration object
 */

#pragma once
#ifndef H_VMCONFIG
#define H_VMCONFIG

#include <string>
#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "VMConfig.h"

class AsmVMConfig
{
protected:
public:
    AsmVMConfig(void);
    virtual ~AsmVMConfig(void);

protected:
    void registerInstruction(std::string name, unsigned char bin, ParamType type = PT_DEFAULT);

public:
    bool isValidInstruction(std::string name);
    bool isValidInstruction(unsigned char bin);

    unsigned char strToBinary(std::string, ParamType type = PT_DEFAULT);
    std::string binToString(unsigned char bin);
};

#endif

