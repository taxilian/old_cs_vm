#include "VirtualMachine.h"
#include <iostream>
#include <boost/make_shared.hpp>
#include <conio.h>

VirtualMachine::VirtualMachine(void) : m_config(boost::make_shared<VMConfig>())
{
    reset();

    registerHandler(m_config->strToBinary("ADD"), make_method(this, &VirtualMachine::ADD));
    registerHandler(m_config->strToBinary("ADI"), make_method(this, &VirtualMachine::ADI));
    registerHandler(m_config->strToBinary("SUB"), make_method(this, &VirtualMachine::SUB));
    registerHandler(m_config->strToBinary("MUL"), make_method(this, &VirtualMachine::MUL));
    registerHandler(m_config->strToBinary("DIV"), make_method(this, &VirtualMachine::DIV));

    registerHandler(m_config->strToBinary("JMP"), make_method(this, &VirtualMachine::JMP));
    registerHandler(m_config->strToBinary("JMR"), make_method(this, &VirtualMachine::JMR));
    registerHandler(m_config->strToBinary("BRZ"), make_method(this, &VirtualMachine::BRZ));
    registerHandler(m_config->strToBinary("BNZ"), make_method(this, &VirtualMachine::BNZ));
    registerHandler(m_config->strToBinary("BLT"), make_method(this, &VirtualMachine::BLT));
    registerHandler(m_config->strToBinary("BGT"), make_method(this, &VirtualMachine::BGT));

    registerHandler(m_config->strToBinary("LD"), make_method(this, &VirtualMachine::LD));
    registerHandler(m_config->strToBinary("LDA"), make_method(this, &VirtualMachine::LDA));
    registerHandler(m_config->strToBinary("LDR"), make_method(this, &VirtualMachine::LDR));
    registerHandler(m_config->strToBinary("STR"), make_method(this, &VirtualMachine::STR));
    registerHandler(m_config->strToBinary("STB"), make_method(this, &VirtualMachine::STB));
    registerHandler(m_config->strToBinary("LDB"), make_method(this, &VirtualMachine::LDB));
    registerHandler(m_config->strToBinary("MOV"), make_method(this, &VirtualMachine::MOV));

    registerHandler(m_config->strToBinary("AND"), make_method(this, &VirtualMachine::AND));
    registerHandler(m_config->strToBinary("OR"), make_method(this, &VirtualMachine::OR));
    registerHandler(m_config->strToBinary("CMP"), make_method(this, &VirtualMachine::CMP));

    registerHandler(m_config->strToBinary("TRP"), make_method(this, &VirtualMachine::TRP));
}

// Used to register a handler for the given instruction
void VirtualMachine::registerHandler(unsigned char inst, CallInstructionFunctor func)
{
    m_functionMap[inst] = func;
}

// This function analyzes the instruction and calls the correct handler for it
void VirtualMachine::callHandler(unsigned int instruction)
{
    instructionBlock blk;
    blk.value = instruction;
    std::map<unsigned char, CallInstructionFunctor>::iterator fnd = m_functionMap.find(blk.instruction);
    if (fnd != m_functionMap.end()) {
        (fnd->second)(blk);
    } else {
        throw std::exception("Invalid instruction!");
    }
}

VirtualMachine::~VirtualMachine(void)
{
}

// Resets the machine to its initial state
void VirtualMachine::reset()
{
    for (int i = 0; i < 16; i++)
        reg[i] = 0;
    m_running = false;
}

// Loads the program block into memory
void VirtualMachine::load(boost::shared_array<unsigned int> block)
{
    m_running = false;
    m_block = block;
}

// This is the main system loop
void VirtualMachine::run(unsigned short start)
{
    pc = start;
    m_running = true;

#ifdef DEBUG
    std::cout << "Beginning to run program." << std::endl;
#endif
    while (m_running) {
        int line = this->byteToLineMap[pc];
        callHandler(m_block[pc++]);
    };
}

// Returns a string with the line number of the instruction at the given address
std::string VirtualMachine::getDebugFor(unsigned short addr)
{
    int line = byteToLineMap[addr];
    return std::string("Error occurred on line " + byteToLineMap[addr]);
}

void VirtualMachine::ADD(REGISTER &rd, REGISTER &rs) { rd += rs; }
void VirtualMachine::ADI(REGISTER &rd, IMMEDIATE val) { rd += val; }
void VirtualMachine::SUB(REGISTER &rd, REGISTER &rs) { rd -= rs; }
void VirtualMachine::MUL(REGISTER &rd, REGISTER &rs) { rd *= rs; }
void VirtualMachine::DIV(REGISTER &rd, REGISTER &rs) { rd /= rs; }

void VirtualMachine::LD(REGISTER &rd, REGISTER &rs) { rd = m_block[rs]; }
void VirtualMachine::LDA(REGISTER &rd, ADDRESS addr) { rd = addr; }
void VirtualMachine::LDB(REGISTER &rd, ADDRESS addr) { rd = m_block[addr]; }
void VirtualMachine::STB(REGISTER &rs, ADDRESS addr) { m_block[addr] = (unsigned char)rs; }
void VirtualMachine::LDR(REGISTER &rd, ADDRESS addr) { rd = m_block[addr]; }
void VirtualMachine::STR(REGISTER &rs, ADDRESS addr) { m_block[addr] = rs; }
void VirtualMachine::MOV(REGISTER &rd, REGISTER &rs) { rd = rs; }

void VirtualMachine::JMP(ADDRESS addr)
{
#ifdef DEBUG
    if (addr == 0) {
        throw VMException(getDebugFor(pc));
    }
#endif
    pc = addr;
}
void VirtualMachine::JMR(REGISTER &r) { JMP(r); }
void VirtualMachine::BRZ(REGISTER &r, ADDRESS addr) { if (r == 0) JMP(addr); }
void VirtualMachine::BNZ(REGISTER &r, ADDRESS addr) { if (r != 0) JMP(addr); }
void VirtualMachine::BLT(REGISTER &r, ADDRESS addr) { if (r < 0) JMP(addr); }
void VirtualMachine::BGT(REGISTER &r, ADDRESS addr) { if (r > 0) JMP(addr); }

void VirtualMachine::AND(REGISTER &rd, REGISTER &rs) { rd = (rs && rd) ? 1 : 0; }
void VirtualMachine::OR(REGISTER &rd, REGISTER &rs) { rd = (rs || rd) ? 1 : 0; }
void VirtualMachine::CMP(REGISTER &rd, REGISTER &rs) { rd = (rd > rs) ? 1 : (rd < rs ? -1 : 0); }

void VirtualMachine::TRP(IMMEDIATE i)
{
    switch(i) {
    case 0:
        pc = 0;
        m_running = false;
        break;
    case 1:
        std::cout << (int)reg[0];
        break;
    case 2:
        std::cin >> reg[8];
        break;
    case 3:
        std::cout << (char)reg[8];
        break;
    case 4:
        reg[8] = _getch();
        break;
    default:
        throw std::exception("Invalid TRP statement!");
    }
}