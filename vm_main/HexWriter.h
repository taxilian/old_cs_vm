#pragma once

#include "VMCore.h"
#include <string>

using boost::uint8_t;

namespace VM {
    void WriteToHex( const std::string& fileName, uint8_t* data, const size_t length, const uint32_t addr, const uint32_t offset = 0 );
    void ReadFromHex( const std::string& fileName, VM::MemoryBlock& data, size_t& length, uint32_t& addr, uint32_t& offset);
}