#include "VirtualMachine.h"
#include <iostream>
#include <boost/make_shared.hpp>
#include <conio.h>

#ifdef _DEBUG
#define TRACEON
#endif

#ifdef TRACEON
#define DOC(FUNC, PARAM1, PARAM2) std::cerr << FUNC << ": " << PARAM1 << ", " << PARAM2;
#define LOG(MSG) std::cerr << MSG << std::endl;
#else
#define DOC(FUNC, PARAM1, PARAM2)
#define LOG(MSG)
#endif

VirtualMachine::VirtualMachine(void) : m_config(boost::make_shared<VMConfig>())
{
    reset();

    registerHandler("ADD", make_method(this, &VirtualMachine::ADD));
    registerHandler("ADI", make_method(this, &VirtualMachine::ADI));
    registerHandler("SUB", make_method(this, &VirtualMachine::SUB));
    registerHandler("MUL", make_method(this, &VirtualMachine::MUL));
    registerHandler("DIV", make_method(this, &VirtualMachine::DIV));

    registerHandler("JMP", make_method(this, &VirtualMachine::JMP));
    registerHandler("JMR", make_method(this, &VirtualMachine::JMR));
    registerHandler("BRZ", make_method(this, &VirtualMachine::BRZ));
    registerHandler("BNZ", make_method(this, &VirtualMachine::BNZ));
    registerHandler("BLT", make_method(this, &VirtualMachine::BLT));
    registerHandler("BGT", make_method(this, &VirtualMachine::BGT));

    registerHandler("LDA", make_method(this, &VirtualMachine::LDA));
    registerHandler("LDR", make_method(this, &VirtualMachine::LDR));
    registerHandler("LDR", make_method(this, &VirtualMachine::LDR2));
    registerHandler("STR", make_method(this, &VirtualMachine::STR));
    registerHandler("STR", make_method(this, &VirtualMachine::STR2));
    registerHandler("STB", make_method(this, &VirtualMachine::STB));
    registerHandler("STB", make_method(this, &VirtualMachine::STB2));
    registerHandler("LDB", make_method(this, &VirtualMachine::LDB));
    registerHandler("LDB", make_method(this, &VirtualMachine::LDB2));
    registerHandler("MOV", make_method(this, &VirtualMachine::MOV));

    registerHandler("AND", make_method(this, &VirtualMachine::AND));
    registerHandler("OR", make_method(this, &VirtualMachine::OR));
    registerHandler("CMP", make_method(this, &VirtualMachine::CMP));

    registerHandler("TRP", make_method(this, &VirtualMachine::TRP));
}

// Used to register a handler for the given instruction
void VirtualMachine::registerHandler(std::string name, instructionDef func)
{
    unsigned char inst = m_config->strToBinary(name, func.type);
    m_functionMap[inst] = func;
}

// This function analyzes the instruction and calls the correct handler for it
void VirtualMachine::callHandler(unsigned int instruction)
{
    instructionBlock blk;
    blk.value = instruction;
    FunctionMap::iterator fnd = m_functionMap.find(blk.instruction);
    if (fnd != m_functionMap.end()) {
        fnd->second.func(blk);
    } else {
        std::string str("Invalid instruction @ ");
        str += (pc - 4);
        str += " (line " + byteToLineMap[pc-4];
        str += "!";
        throw std::exception(str.c_str());
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
void VirtualMachine::load(boost::shared_array<unsigned char> block)
{
    m_running = false;
    m_block = block;
}

// This is the main system loop
void VirtualMachine::run(unsigned short start)
{
    pc = start;
    m_running = true;

#ifdef TRACEON
    std::cerr << "Beginning to run program." << std::endl;
#endif
    while (m_running) {
        int line = this->byteToLineMap[pc];
        int addr = pc;
#ifdef TRACEON
        std::cerr << line << " @ " << addr << ":\t";
#endif
        int rl = get_int(pc);
        pc += 4;
        callHandler(rl);
#ifdef TRACEON
        std::map<unsigned short, std::string>::iterator fnd = labelReverse.find(addr);
        if (fnd != labelReverse.end())
            std::cerr << "\t(" << fnd->second.c_str() << ")";
        std::cerr << std::endl;
#endif
    };
}

// Returns a string with the line number of the instruction at the given address
std::string VirtualMachine::getDebugFor(unsigned short addr)
{
    int line = byteToLineMap[addr];
    return std::string("Error occurred on line " + byteToLineMap[addr]);
}

int VirtualMachine::get_int(unsigned short addr)
{
    return *((int*)(&m_block[addr]));
}

void VirtualMachine::set_int(unsigned short addr, int value)
{
    *((int*)(&m_block[addr])) = value;
}

char VirtualMachine::get_byte(unsigned short addr)
{
    return *((char*)(&m_block[addr]));
}

void VirtualMachine::set_byte(unsigned short addr, char value)
{
    *((char*)(&m_block[addr])) = value;
}

void VirtualMachine::ADD(REGISTER &rd, REGISTER &rs) 
{
    DOC("ADD", rd, rs); rd += rs;
}
void VirtualMachine::ADI(REGISTER &rd, IMMEDIATE val)
{
    DOC("ADI", rd, val); rd += val;
}
void VirtualMachine::SUB(REGISTER &rd, REGISTER &rs)
{
    DOC("SUB", rd, rs); rd -= rs; 
}
void VirtualMachine::MUL(REGISTER &rd, REGISTER &rs)
{
    DOC("MUL", rd, rs); rd *= rs; 
}
void VirtualMachine::DIV(REGISTER &rd, REGISTER &rs)
{
    DOC("DIV", rd, rs); rd /= rs; 
}

void VirtualMachine::LDA(REGISTER &rd, ADDRESS addr) 
{
    DOC("LDA", "R" << static_cast<int>(&rd - this->reg), addr);
    rd = addr;
}
void VirtualMachine::LDR(REGISTER &rd, ADDRESS addr)
{
    DOC("LDR", "R" << static_cast<int>(&rd - this->reg), addr << " -> " << get_int(addr));
    rd = get_int(addr);
}
void VirtualMachine::LDR2(REGISTER &rd, REGISTER &rs)
{
    DOC("LDR2", "R" << static_cast<int>(&rd - this->reg), rs << " -> " << get_int(rs));
    rd = get_int(rs);
}
void VirtualMachine::LDB(REGISTER &rd, ADDRESS addr)
{
    DOC("LDB", "R" << static_cast<int>(&rd - this->reg), addr << " -> " << (int)get_byte(addr) << "(" << get_byte(addr) << ")");
    rd = get_byte(addr);
}
void VirtualMachine::LDB2(REGISTER &rd, REGISTER &rs)
{
    DOC("LDB2", "R" << static_cast<int>(&rd - this->reg), rs << " -> " << (int)get_byte(rs) << "(" << get_byte(rs) << ")");
    rd = get_byte(rs);
}
void VirtualMachine::STB(REGISTER &rs, ADDRESS addr)
{
    DOC("STB", rs, addr); set_byte(addr, (char)rs);
}
void VirtualMachine::STB2(REGISTER &rs, REGISTER &rd)
{
    DOC("STB2", rs, rd); set_byte(rd, (char)rs);
}
void VirtualMachine::STR(REGISTER &rs, ADDRESS addr)
{
    DOC("STR", rs, addr); set_int(addr, rs);
}
void VirtualMachine::STR2(REGISTER &rs, REGISTER &rd)
{
    DOC("STR2", rs, rd); set_int(rd, rs);
}
void VirtualMachine::MOV(REGISTER &rd, REGISTER &rs)
{
    DOC("MOV", "R" << static_cast<int>(&rd - this->reg), rs);
    rd = rs;
}

void VirtualMachine::JMP(ADDRESS addr)
{
    DOC("JMP", addr, this->byteToLineMap[addr]);
#ifdef DEBUG
    if (addr == 0) {
        throw VMException(getDebugFor(pc));
    }
#endif
    pc = addr;
}
void VirtualMachine::JMR(REGISTER &r)
{
    DOC("JMR", r, "-"); JMP(r); 
}
void VirtualMachine::BRZ(REGISTER &r, ADDRESS addr)
{
    DOC("BRZ", r, addr); if (r == 0) JMP(addr);
}
void VirtualMachine::BNZ(REGISTER &r, ADDRESS addr)
{
    DOC("BNZ", r, addr); if (r != 0) JMP(addr); 
}
void VirtualMachine::BLT(REGISTER &r, ADDRESS addr)
{
    DOC("BLT", r, addr); if (r < 0) JMP(addr);
}
void VirtualMachine::BGT(REGISTER &r, ADDRESS addr)
{
    DOC("BGT", r, addr); if (r > 0) JMP(addr);
}

void VirtualMachine::AND(REGISTER &rd, REGISTER &rs)
{
    DOC("AND", rd, rs); rd = (rs && rd) ? 1 : 0;
}
void VirtualMachine::OR(REGISTER &rd, REGISTER &rs)
{
    DOC("OR", rd, rs); rd = (rs || rd) ? 1 : 0; 
}
void VirtualMachine::CMP(REGISTER &rd, REGISTER &rs)
{
    DOC("CMP", rd, rs); rd = (rd > rs) ? 1 : (rd < rs ? -1 : 0); 
}

void VirtualMachine::TRP(IMMEDIATE i)
{
    DOC("TRP", i, "-"); 
    switch(i) {
    case 0:
        pc = 0;
        LOG(" Program halted");
        m_running = false;
        break;
    case 1:
        LOG(" Writing integer: " << reg[0]);
        std::cout << (int)reg[0];
        break;
    case 2:
        std::cin >> reg[8];
        LOG(" Received integer: " << reg[0])
        break;
    case 3:
        LOG(" Writing char: \"" << (char)reg[8] << "\"");
        if (reg[8] == '\n') {
            std::cout << std::endl;
        } else {
            std::cout << (char)reg[8];
        }
        break;
    case 4:
        reg[8] = _getch();
        if (reg[8] == '\r')
            reg[8] = '\n';
        LOG(" Received character " << reg[8] << "('" << (char)reg[8] << "')");
        break;
    default:
        throw std::exception("Invalid TRP statement!");
    }
}

void VirtualMachine::setDebugInfo( std::map<unsigned short, int>& linemap, std::map<unsigned short, std::string> &revLabelMap )
{
    byteToLineMap = linemap;
    labelReverse = revLabelMap;
}
