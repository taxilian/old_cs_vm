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
    registerInstruction("BLT", 0x05);
    registerInstruction("BRZ", 0x06);

    // Move Instructions
    registerInstruction("MOV", 0x11);
    registerInstruction("LDA", 0x12);
    registerInstruction("STR", 0x13);
    registerInstruction("LDR", 0x14);
    registerInstruction("STB", 0x15);
    registerInstruction("LDB", 0x16);
    registerInstruction("LD",  0x17);
    
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

void VMConfig::registerInstruction(std::string name, unsigned char bin)
{
    Instruction ins = {name, bin};
    strMap[name] = bin;
    binMap[bin] = ins;
}

unsigned char VMConfig::strToBinary(std::string name)
{
    return strMap[name];
}

std::string VMConfig::binToString(unsigned char bin)
{
    return binMap[bin].name;
}