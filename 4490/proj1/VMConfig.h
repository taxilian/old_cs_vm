/**
 * VMConfig.h
 *
 * Richard Bateman
 * Virtual Machine Configuration object
 */

#pragma once
#ifndef H_VMCONFIG
#define H_VMCONFIG

#include <string>
#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>

union instructionBlock {
    unsigned int value;
    struct {
        unsigned char instruction;
        unsigned char uint8_param;
        union {
            unsigned short uint16_param2;
            struct {
                unsigned char uint8_param2;
                unsigned char uint8_param3;
            };
        };
        
    };
};

class VMConfig;
typedef boost::shared_ptr<VMConfig> VMConfigPtr;
class VMConfig
{
protected:
    struct Instruction
    {
        std::string name;
        unsigned char bin;
    };

public:
    VMConfig(void);
    virtual ~VMConfig(void);

protected:
    void registerInstruction(std::string, unsigned char);

public:
    bool isValidInstruction(std::string);
    bool isValidInstruction(unsigned char);

    unsigned char strToBinary(std::string);
    std::string binToString(unsigned char);

protected:
    std::map<std::string, unsigned char> strMap;
    std::map<unsigned char, Instruction> binMap;
};

#endif
