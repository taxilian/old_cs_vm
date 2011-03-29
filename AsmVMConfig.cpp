#include "AsmVMConfig.h"


AsmVMConfig::AsmVMConfig(void)
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

    // Threading
    registerInstruction("RUN", 0x41);
    registerInstruction("END", 0x42);
    registerInstruction("BLK", 0x43);
    registerInstruction("LCK", 0x44);
    registerInstruction("ULK", 0x45);
}


AsmVMConfig::~AsmVMConfig(void)
{
}

