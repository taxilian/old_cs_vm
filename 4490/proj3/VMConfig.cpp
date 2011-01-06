#include "VMConfig.h"


VMConfig::VMConfig(void)
{
    // Trap
    registerInstruction("TRP", 0x88);

    // Jump instructions
    registerInstruction("JMP", 0x01);
    registerInstruction("JMR", 0x02);
    registerInstruction("BNZ", 0x03);
    registerInstruction("BNZ", 0x04);
    registerInstruction("BGT", 0x05);
    registerInstruction("BLT", 0x06);
    registerInstruction("BRZ", 0x07);

    // Move Instructions
    registerInstruction("MOV", 0x11);
    registerInstruction("LDA", 0x12);
    registerInstruction("STR", 0x13, PT_REGADDR);
    registerInstruction("STR", 0x14, PT_REGREG);
    registerInstruction("LDR", 0x15, PT_REGADDR);
    registerInstruction("LDR", 0x16, PT_REGREG);
    registerInstruction("STB", 0x17, PT_REGADDR);
    registerInstruction("STB", 0x18, PT_REGREG);
    registerInstruction("LDB", 0x19, PT_REGADDR);
    registerInstruction("LDB", 0x1A, PT_REGREG);
    
    // Arithmetic instructions
    registerInstruction("ADD", 0x20);
    registerInstruction("ADI", 0x21);
    registerInstruction("SUB", 0x22);
    registerInstruction("MUL", 0x23);
    registerInstruction("DIV", 0x24);

    // Logical Instructions
    registerInstruction("AND", 0x31);
    registerInstruction("OR",  0x32);
    // Compare
    registerInstruction("CMP", 0x39);
}


VMConfig::~VMConfig(void)
{
}

bool VMConfig::isValidInstruction(std::string name)
{
    return strMap.find(name) != strMap.end();
}

bool VMConfig::isValidInstruction(unsigned char bin)
{
    return binMap.find(bin) != binMap.end();
}

void VMConfig::registerInstruction(std::string name, unsigned char bin, ParamType type)
{
    opcode ins = {name, bin, type};
    strMap.insert(std::pair<std::string, opcode>(name, ins));
    binMap[bin] = name;
}

unsigned char VMConfig::strToBinary(std::string name, ParamType type)
{
    std::pair<MMapType::iterator, MMapType::iterator> range = strMap.equal_range(name);
    opcode found;

    for (MMapType::iterator fnd = range.first; fnd != range.second; fnd++) {
        found = fnd->second;
        if (found.type == type || type == PT_DEFAULT) {
            return found.bin;
        }
    }
    return found.bin;
}

std::string VMConfig::binToString(unsigned char bin)
{
    return binMap[bin];
}