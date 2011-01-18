
#include <boost/filesystem.hpp>
#include <iostream>
#include "Assembler.h"
#include "VirtualMachine.h"
#include "HexWriter.h"

int main(int argc, char *argv[] )
{
    namespace fs = boost::filesystem;
    fs::path fullPath(fs::initial_path<fs::path>() );
    try {
        if (argc < 3) {
            std::cout << "Usage: " << argv[0] << " <infile>.asm <outfile>.hexe" << std::endl;
            exit(1);
        }
        Assembler assembler(argv[1]);
        boost::uint32_t startAddr(0);
        startAddr = assembler.start();

        boost::shared_array<unsigned char> data(assembler.getBlock());
        size_t size = assembler.getBlockSize();

        VM::WriteToHex(argv[2], data.get(), size, startAddr);
    } catch (std::exception &ex) {
        std::cout << "Assembler error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}