
#include <iostream>
#include "Assembler.h"
#include "VirtualMachine.h"
int main(int argc, char *argv[] )
{
    Assembler assembler(argv[1]);
    unsigned short startAddr(0);
    try {
        startAddr = assembler.start();
    } catch (std::exception &ex) {
        std::cout << "Assembler error: " << ex.what() << std::endl;
        return 1;
    }

    boost::shared_array<unsigned char> data(assembler.getBlock());
    VirtualMachine vm;
    try {
        vm.setDebugInfo(assembler.byteToLineMap, assembler.labelReverse);
        vm.load(data, assembler.getBlockSize());
        vm.run(startAddr);
    } catch (std::exception &ex) {
        std::cout << "VM error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}