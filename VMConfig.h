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
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/cstdint.hpp>

union instructionBlock {
    boost::int64_t value;
    struct {
        boost::int16_t instruction;
        boost::int16_t uint16_param;
        union {
            boost::uint32_t uint32_param2;
            struct {
                boost::int16_t uint16_param2;
                boost::int16_t uint16_param3;
            };
        };
        
    };
};

typedef boost::function<void (const instructionBlock&)> CallInstructionFunctor;

enum ParamType {
    PT_RAW,
    PT_ADDR,
    PT_IMMEDIATE,
    PT_REG,
    PT_REGREG,
    PT_REGADDR,
    PT_REGIMMEDIATE,
    PT_DEFAULT
};

struct opcode
{
    std::string name;
    unsigned char bin;
    ParamType type;
};

typedef std::multimap<std::string, opcode> MMapType;

struct instructionDef {
    ParamType type;
    CallInstructionFunctor func;
};

class VMConfig;
typedef boost::shared_ptr<VMConfig> VMConfigPtr;
class VMConfig
{
protected:
public:
    VMConfig(void);
    virtual ~VMConfig(void);

protected:
    void registerInstruction(std::string name, unsigned char bin, ParamType type = PT_DEFAULT);

public:
    bool isValidInstruction(std::string name);
    bool isValidInstruction(unsigned char bin);

    unsigned char strToBinary(std::string, ParamType type = PT_DEFAULT);
    std::string binToString(unsigned char bin);

protected:
    MMapType strMap;
    std::map<unsigned char, std::string> binMap;
};

#endif
