#include <fstream>
#include <iomanip>
#include <algorithm>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/casts.hpp>
#include <boost/make_shared.hpp>
#include "VMCore.h"

#include "HexWriter.h"

void VM::WriteToHex( const std::string& fileName, uint8_t* data, const size_t length, const uint32_t addr, const uint32_t offset )
{
    using namespace boost::lambda;
    std::ofstream out(fileName);
    if (!out) {
        throw std::runtime_error("Could not write file " + fileName);
    }

    out << std::hex
        << length << ' '
        << addr << ' '
        << offset << ' ';

    std::for_each(data, data+length,
        out << constant(std::setw(2))
            << std::setfill('0')
            << ll_static_cast<int>(_1)
            << ' ');
    out.close();
}

void VM::ReadFromHex( const std::string& fileName, VM::MemoryBlock& data, size_t& length, uint32_t& addr, uint32_t& offset )
{
    using namespace boost::lambda;
    std::ifstream in(fileName);
    if (!in) {
        throw std::runtime_error("Could not read file " + fileName);
    }

    in >> std::hex >> length;
    in >> std::hex >> addr;
    in >> std::hex >> offset;

    data = VM::MemoryBlock(new uint8_t[length]);
    for (uint8_t* ptr = data.get(); ptr != data.get()+length; ptr++) {
        std::string hex;
        in >> hex;
        *ptr = static_cast<uint8_t>(strtol(hex.c_str(), NULL, 16));
    }
}
