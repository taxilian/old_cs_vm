#include "VirtualMachine.h"
#include <iostream>
#include <boost/make_shared.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/cast.hpp>
#include <conio.h>
#include <boost/mpl/assert.hpp>

#include <boost/random/linear_congruential.hpp>

#ifdef __DEBUG
#define TRACEON
#endif

#ifdef TRACEON
#define DOC(FUNC, PARAM1, PARAM2) std::cerr << FUNC << ": " << PARAM1 << ", " << PARAM2;
#define LOG(MSG) std::cerr << MSG << std::endl;
#else
#define DOC(FUNC, PARAM1, PARAM2)
#define LOG(MSG)
#endif

using namespace VM;

VirtualMachine::VirtualMachine(void) : m_config(boost::make_shared<VMConfig>()), BOUND_CODE(0), offset(0),
    sched_baseTicks(30), sched_variance(0.2)
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
void VirtualMachine::callHandler(boost::uint64_t instruction)
{
    instructionBlock blk;
    blk.value = instruction;
    FunctionMap::iterator fnd = m_functionMap.find(blk.instruction);
    if (fnd != m_functionMap.end()) {
        fnd->second.func(blk);      // Execute the instruction
    } else {
        std::stringstream ss;
        ss << "Invalid instruction @ " << (pc - sizeof(boost::uint64_t)) << " (line ";
        ss << byteToLineMap[static_cast<unsigned int>(pc-4)] << "!";
        throw std::exception(ss.str().c_str());
    }
}

VirtualMachine::~VirtualMachine(void)
{
}

// Resets the machine to its initial state
void VirtualMachine::reset()
{
    this->m_block = MemoryBlock(new unsigned char[MEMORY_SIZE+3]);

    ADDRESS bottom = MEMORY_SIZE - 1;
    memset(reg, 0, sizeof(reg[0])*20);
    m_running = false;

    reg[SB] = MEMORY_SIZE - 1;  // Stack Base
    reg[FP] = reg[SB];          // Frame Pointer (Bottom of current frame)
    reg[SP] = reg[SB];          // Stack Pointer (Top of stack)
    reg[SL] = 0;                // Stack Limit (Top of available stack memory)
}

VM::VMState VM::VirtualMachine::getRegisterState()
{
    VM::VMState state;
    state.pc = pc;
    state.offset = offset;
	state.pid = pid;
    std::copy(reg, &reg[20], state.reg);
    return state;
}

void VM::VirtualMachine::setRegisterState( const VMState& state )
{
    offset = state.offset;
    pc = state.pc;
	pid = state.pid;
    std::copy(state.reg, &state.reg[20], reg);
}

void VM::VirtualMachine::loadProgram( const MemoryBlock& memory, const size_t size, const uint32_t offset /*= 0*/ )
{
    assert((offset + size) < MEMORY_SIZE);

    // Copy the memory in
    std::copy(memory.get(), memory.get() + size, m_block.get() + offset);

    // Allocate stack
    //reg[SL] = offset + size;
    //reg[SB] = reg[SP] = reg[FP] = reg[SL] + stackSize;

    // Program loaded, stack initialized!
}

void VM::VirtualMachine::setMemoryOffset( const uint32_t offset )
{
    this->offset = offset;
}

uint32_t VM::VirtualMachine::getMemorySize()
{
    return MEMORY_SIZE;
}

void VM::VirtualMachine::registerInterrupt( int trap, const VM::InterruptHandler& handler )
{
    osInterrupts[trap] = handler;
}

void VM::VirtualMachine::setRunning( bool isRunning )
{
    m_running = isRunning;
}
bool VM::VirtualMachine::isRunning()
{
    return m_running;
}

void VM::VirtualMachine::configureScheduler( const int baseTicks, const double variance, const InterruptHandler& interrupt )
{
    sched_baseTicks = baseTicks;
    sched_variance = variance;
    sched_interrupt = interrupt;
}

std::string VirtualMachine::getLabelForAddress(ADDRESS addr) {
    std::map<boost::uint32_t, std::string>::iterator fnd = labelReverse.find(addr);
    std::string out;
    if (fnd != labelReverse.end()) {
        out += "\t(";
        out += (fnd->second.c_str());
        out += ")";
        return out;
    } else {
        return "";
    }
}

int VirtualMachine::sched_calcTarget()
{
    double rndBase(rand() / double(RAND_MAX));
    double percentToAdd = rndBase * sched_variance;
    int posOrNeg = (rand() / double(RAND_MAX)) >= 0.5 ? 1 : -1;
    return boost::numeric_cast<int>(sched_baseTicks + (percentToAdd * sched_baseTicks * posOrNeg));
}

// This is the main system loop
Status VirtualMachine::run()
{
    m_running = true;

    int i = 0;
    Status stat;
    int clock(0);
    int schedTarget(sched_calcTarget());
    while (m_running) {
        //int line = this->byteToLineMap[static_cast<unsigned int>(pc)];
        stat = tick();
        if (++clock > schedTarget) {
            // We've hit the target clock cycle to let the scheduler do its thing
            // Reset the clock and call the scheduler
            clock = 0;
            schedTarget = sched_calcTarget();
            // If a scheduler interrupt handler is registered, call the scheduler
            if (sched_interrupt)
                sched_interrupt(this);
        }
    };
    return stat;
}

Status VirtualMachine::tick()
{
    uint64_t addr = pc;
    uint64_t rl = get_int(static_cast<uint32_t>(pc));

    pc += sizeof(pc);
    
    callHandler(rl);

    if (m_running) {
        return Status_Running;
    } else {
        return Status_Stopped;
    }
}


// Returns a string with the line number of the instruction at the given address
std::string VirtualMachine::getDebugFor(boost::uint32_t addr)
{
    int line = byteToLineMap[addr];
    return std::string("Error occurred on line " + byteToLineMap[addr]);
}

boost::uint64_t VirtualMachine::get_int(boost::uint32_t addr)
{
    return *((boost::uint64_t*)(&m_block[addr+offset]));
}

void VirtualMachine::set_int(boost::uint32_t addr, boost::uint64_t value)
{
    *((boost::uint64_t*)(&m_block[addr+offset])) = value;
}

char VirtualMachine::get_byte(boost::uint32_t addr)
{
    return *((char*)(&m_block[addr+offset]));
}

void VirtualMachine::set_byte(boost::uint32_t addr, char value)
{
    *((char*)(&m_block[addr+offset])) = value;
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
    DOC("LDR", "R" << static_cast<int>(&rd - this->reg), addr << " -> " << get_int((boost::uint32_t)addr));
    rd = get_int((boost::uint32_t)addr);
}
void VirtualMachine::LDR2(REGISTER &rd, REGISTER &rs)
{
    DOC("LDR2", "R" << static_cast<int>(&rd - this->reg), rs << " -> " << get_int((boost::uint32_t)rs));
    rd = get_int((boost::uint32_t)rs);
}
void VirtualMachine::LDB(REGISTER &rd, ADDRESS addr)
{
    DOC("LDB", "R" << static_cast<int>(&rd - this->reg), addr << " -> " << (int)get_byte((boost::uint32_t)addr) << "(" << get_byte((boost::uint32_t)addr) << ")");
    rd = get_byte((boost::uint32_t)addr);
}
void VirtualMachine::LDB2(REGISTER &rd, REGISTER &rs)
{
    DOC("LDB2", "R" << static_cast<int>(&rd - this->reg), rs << " -> " << (int)get_byte((boost::uint32_t)rs) << "(" << get_byte((boost::uint32_t)rs) << ")");
    rd = get_byte((boost::uint32_t)rs);
}
void VirtualMachine::STB(REGISTER &rs, ADDRESS addr)
{
    DOC("STB", rs, addr); set_byte(addr, (char)rs);
}
void VirtualMachine::STB2(REGISTER &rs, REGISTER &rd)
{
    DOC("STB2", rs, rd); set_byte((boost::uint32_t)rd, (char)rs);
}
void VirtualMachine::STR(REGISTER &rs, ADDRESS addr)
{
    DOC("STR", rs, addr); set_int(addr, rs);
}
void VirtualMachine::STR2(REGISTER &rs, REGISTER &rd)
{
    DOC("STR2", rs, rd); set_int((boost::uint32_t)rd, rs);
}
void VirtualMachine::MOV(REGISTER &rd, REGISTER &rs)
{
    DOC("MOV", "R" << static_cast<int>(&rd - this->reg), rs);
    rd = rs;
}

void VirtualMachine::JMP(ADDRESS addr)
{
    DOC("JMP", addr << this->getLabelForAddress(addr), this->byteToLineMap[addr]);
#ifdef DEBUG
    if (addr == 0) {
        throw VMException(getDebugFor(pc));
    }
#endif
    pc = addr;
}
void VirtualMachine::JMR(REGISTER &r)
{
    DOC("JMR", r, "-"); JMP(static_cast<ADDRESS>(r)); 
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
        std::cin >> reg[0];
        LOG(" Received integer: " << reg[0]);
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
        std::cout << reg[8];
        LOG(" Received character " << reg[8] << "('" << (char)reg[8] << "')");
        break;
    default:
        VM::InterruptTable::const_iterator fnd = osInterrupts.find(i);
        if (fnd != osInterrupts.end()) {
            // look it up in the table; if it exists, call it with a pointer to the VMCore* interface
            fnd->second(this);
        } else {
            throw std::exception("Invalid TRP statement!");
        }
        break;
    }
}
 
void VirtualMachine::setDebugInfo( std::map<boost::uint32_t, int>& linemap, std::map<boost::uint32_t, std::string> &revLabelMap )
{
    byteToLineMap = linemap;
    labelReverse = revLabelMap;
}
