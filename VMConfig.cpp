#include "VMConfig.h"

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

