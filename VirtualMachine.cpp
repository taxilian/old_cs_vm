#include "VirtualMachine.h"
#include "AsmVMConfig.h"
#include <iostream>
#include <boost/make_shared.hpp>
#include <boost/lexical_cast.hpp>

#ifdef _DEBUG
#define TRACEON
#endif

#ifdef TRACEON
#define DOC(FUNC, PARAM1, PARAM2) std::cerr << FUNC << ": " << PARAM1 << ", " << PARAM2;
#define LOG(MSG) std::cerr << MSG << std::endl; std::cerr.flush();
#else
#define DOC(FUNC, PARAM1, PARAM2)
#define LOG(MSG)
#endif

#define MEMORY_SIZE 16384

VirtualMachine::VirtualMachine(void) : m_config(boost::make_shared<AsmVMConfig>()), BOUND_CODE(0), m_curThread(-1), threadCount(0), m_blocked(false)
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

    registerHandler("RUN", make_method(this, &VirtualMachine::RUN));
    registerHandler("END", make_method(this, &VirtualMachine::END));
    registerHandler("BLK", make_method(this, &VirtualMachine::BLK));
    registerHandler("LCK", make_method(this, &VirtualMachine::LCK));
    registerHandler("ULK", make_method(this, &VirtualMachine::ULK));
    
    void RUN(REGISTER &rd, ADDRESS addr);
    void END();
    void BLK();
    void LCK(ADDRESS addr);
    void ULK(ADDRESS addr);
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
        throw VMException(str.c_str());
    }
}

VirtualMachine::~VirtualMachine(void)
{
}

// Resets the machine to its initial state
void VirtualMachine::reset()
{
    this->m_block = boost::shared_array<unsigned char>(new unsigned char[MEMORY_SIZE+3]);

    for (int i = 0; i < REGISTER_COUNT; i++) {
        reg[i] = 0;
    }
    memset(threadList, 0, sizeof(thread) * THREAD_NUM);
    m_running = false;
    m_blocked = false;

    reg[SB] = MEMORY_SIZE - 1;  // Stack Base
    reg[FP] = reg[SB];          // Frame Pointer (Bottom of current frame)
    reg[SP] = reg[SB];          // Stack Pointer (Top of stack)
    reg[SL] = 0;                // Stack Limit (Top of available stack memory)
    reg[HP] = 0;
    m_curThread = -1;
}

void VirtualMachine::initThread( int threadId, ADDRESS newPC )
{
    LOG("Starting thread " << threadId);
    threadCount++;
    int oldThread = m_curThread;
    changeContext(-1);
    pc = newPC;
    thread* ct(&this->threadList[threadId]);
    if (ct->active)
        throw VMException("Thread already active!");
    reg[SB] = reg[FP] = reg[SP] = ct->SB;
    reg[SL] = ct->SL;
    reg[HP] = ct->SL;
    writeThreadRegisters(threadId);
    set_int(ct->MEMB, newPC);
    ct->active = true;
    changeContext(oldThread);
}

void VirtualMachine::endThread( int threadId )
{
    threadList[threadId].active = false;
    threadCount--;
    LOG("Ending thread " << threadId);
}

void VirtualMachine::changeContext( int newThreadId )
{
    if (m_curThread > -1)
        writeThreadRegisters(m_curThread);
    if (newThreadId > -1)
        loadThreadRegisters(newThreadId);
    m_curThread = newThreadId;
}

int VirtualMachine::getNextAvailableThread()
{
    for(int i = 0; i < THREAD_NUM; i++) {
        if (!threadList[i].active)
            return i;
    }
    return -1;
}

void VirtualMachine::switchToNextThread()
{
    if (m_curThread < 0) m_curThread = 0;
    int i = m_curThread;
    while (true) {
        if (++i == THREAD_NUM) i = 0;
        if (threadList[i].active) {
            changeContext(i);
            return;
        }
    }
}

void VirtualMachine::writeThreadRegisters(int threadId)
{
    thread* ct(&this->threadList[threadId]);
    this->set_int(ct->MEMB, pc);
    for (int i = 0; i < REGISTER_COUNT; i++) {
        this->set_int(ct->MEMB - (1 + i) * sizeof(REGISTER), reg[i]);
    }
}

void VirtualMachine::loadThreadRegisters( int threadId )
{
    thread* ct(&this->threadList[threadId]);
    pc = this->get_int(ct->MEMB);
    for (int i = 0; i < REGISTER_COUNT; i++) {
        reg[i] = this->get_int(ct->MEMB - (1 + i) * sizeof(REGISTER));
    }
}


// Loads the program block into memory
void VirtualMachine::load(boost::shared_array<unsigned char> block, unsigned short size)
{
    memcpy(m_block.get(), block.get(), size);
    BOUND_CODE = size;
    int regCount = REGISTER_COUNT + 1; // REGISTER_COUNT registers plus PC
    int regBlockSize = sizeof(REGISTER) * regCount;
    int stacksize = (MEMORY_SIZE - BOUND_CODE - 1) / 2;
    int threadStackSize = stacksize / this->THREAD_NUM;
    for (int i = 0; i < THREAD_NUM; i++) {
        threadList[i].MEMB = (MEMORY_SIZE - 1) - (threadStackSize * i);
        threadList[i].SB = this->threadList[i].MEMB - regBlockSize;
        threadList[i].SL = MEMORY_SIZE - (threadStackSize * (i+1));
    }
    m_running = false;
}

std::string VirtualMachine::getLabelForAddress(ADDRESS addr) {
    std::map<unsigned short, std::string>::iterator fnd = labelReverse.find(addr);
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

// This is the main system loop
void VirtualMachine::run(unsigned short start)
{
    initThread(0, start);
    changeContext(0);
    pc = start;
    m_running = true;


#ifdef TRACEON
    std::cerr << "Beginning to run program." << std::endl;
#endif
    int i = 0;
    while (m_running) {
        if (++i % 15 == 0 || m_blocked) {
            i = 0;
            switchToNextThread();
            m_blocked = false;
        }
        int line = this->byteToLineMap[pc];
        int addr = pc;
#ifdef TRACEON
        std::cerr << "Thread " << m_curThread << ": " << line << " @ " << addr << ":\t";
#endif
        int rl = get_int(pc);
        pc += 4;
        callHandler(rl);
#ifdef TRACEON
        std::cerr << this->getLabelForAddress(addr) << std::endl;
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
    DOC("ADI", "R" << static_cast<int>(&rd - this->reg) << "(" << rd << ")", val); rd += val;
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
    DOC("LDR", "R" << static_cast<int>(&rd - this->reg), addr << " (" << get_int(addr) << "");
    rd = get_int(addr);
}
void VirtualMachine::LDR2(REGISTER &rd, REGISTER &rs)
{
    DOC("LDR2", "R" << static_cast<int>(&rd - this->reg), "was " << rd << " -> to " << get_int(rs) << " from address " << rs);
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
    DOC("STR2", "R" << static_cast<int>(&rs-reg), rs << " -> R" << static_cast<int>(&rd-reg) << " (was " << rd << ")");
    set_int(rd, rs);
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
        reg[8] = getchar();
        if (reg[8] == '\r')
            reg[8] = '\n';
        std::cout << reg[8];
        LOG(" Received character " << reg[8] << "('" << (char)reg[8] << "')");
        break;
    default:
        throw VMException("Invalid TRP statement!");
    }
}

void VirtualMachine::setDebugInfo( std::map<unsigned short, int>& linemap, std::map<unsigned short, std::string> &revLabelMap )
{
    byteToLineMap = linemap;
    labelReverse = revLabelMap;
}

void VirtualMachine::RUN( REGISTER &rd, ADDRESS addr )
{
    DOC("RUN", rd, addr << this->getLabelForAddress(addr));
    rd = this->getNextAvailableThread();
    if (rd < 0)
        throw VMException("Out of threads");
    this->initThread(rd, addr);
}

void VirtualMachine::END()
{
    DOC("END", "Thread ", boost::lexical_cast<std::string>(m_curThread));
    if (m_curThread == 0) {
        throw VMException("Attempt to call END from main thread");
    }
    this->endThread(m_curThread);
    this->changeContext(0);
}

void VirtualMachine::BLK()
{
    if (m_curThread != 0)
        throw VMException("Attempt to call BLK from thread other than main");
    else if (threadCount > 1) {
        pc -= 4;
        m_blocked = true;
        DOC("BLK", "-", "(Blocking)");
    }
}

void VirtualMachine::LCK( ADDRESS addr )
{
    if (get_byte(addr) == -1) {
        DOC("LCK", getLabelForAddress(addr), "-");
        set_byte(addr, m_curThread);
    } else if (get_byte(addr) != m_curThread) {
        DOC("LCK", this->getLabelForAddress(addr), "(MUTEX IN USE, BLOCKING)");
        pc -= 4;
    m_blocked = true;
    }
}

void VirtualMachine::ULK( ADDRESS addr )
{
    DOC("ULK", getLabelForAddress(addr), "-");
    if (get_byte(addr) == m_curThread) {
        set_byte(addr, -1);
    } else {
        std::string ex("Attempt by thread ");
        ex += m_curThread;
        ex += " to unlock mutex owned by thread " + get_byte(addr);
        throw VMException(ex);
    }
}
