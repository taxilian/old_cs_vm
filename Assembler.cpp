#include "Assembler.h"
#include <boost/make_shared.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include "AsmVMConfig.h"

char Assembler::getRegister(std::string name)
{
    if (name == "PC") {
        return (char)0x7F;
    } else if (name  == "FP") {
        return 16;
    } else if (name  == "SL") {
        return 17;
    } else if (name  == "SP") {
        return 18;
    } else if (name  == "SB") {
        return 19;
    } else if (name  == "HP") {
        return 20;
    } else if ((name[0] != 'R' && name[0] != 'r') || name.size() > 3)
        return -1;
    try {
        return (char)boost::lexical_cast<int>(name.substr(1, name.size() - 1));
    } catch (...) {
        return -1;
    }
}

Assembler::Assembler(std::string filename) : m_config(boost::make_shared<AsmVMConfig>()),
    m_parser(filename, m_config), m_block(new unsigned char[1024]), m_size(0)
{
}

unsigned short Assembler::start()
{
    unsigned short start = 0;
    Parser::LinePtr curLine;
    std::map<std::string, unsigned short> labelAddr;
    std::deque<Parser::LinePtr> lines;
    unsigned short curAddr(0);
#ifdef DEBUG
    std::cout << "Analyzing code..." << std::endl;
#endif
    while (curLine = m_parser.getNextLine()) {
        if (boost::dynamic_pointer_cast<Parser::Comment>(curLine))
            continue; // ignore comments
        std::string lbl(curLine->label);
        if (!curLine->label.empty()) {
            if (labelAddr.find(curLine->label) != labelAddr.end()) {
                std::string msg("Duplicate label: ");
                msg += curLine->label;
                msg += " on line ";
                msg += boost::lexical_cast<std::string>(m_parser.getLineNumber());
                throw AssemblerException(msg);
            }
            labelAddr[curLine->label] = curAddr;
            this->labelReverse[curAddr] = curLine->label;
        }
        lines.push_back(curLine);
        byteToLineMap[curAddr] = (int)m_parser.getLineNumber();
        if (boost::dynamic_pointer_cast<Parser::Byte>(curLine)) {
            curAddr++;
        } else {
            curAddr+=4;
        }
    }
#ifdef DEBUG
    std::cout << "Processed: " << _m_parser.getLineNumber() << " lines. " << labelAddr.size() << " labels, "
        << lines.size() * 4 << " bytes used." << std::endl;
    std::cout << "Building binary block..." << std::endl;
#endif

    m_size = curAddr;
    m_block = boost::shared_array<unsigned char>(new unsigned char[m_size+1]);
    curAddr = 0;
    while (lines.size()) {
        int lineNo = byteToLineMap[curAddr];
        Parser::LinePtr line(lines.front());
        lines.pop_front();
        Parser::IntPtr intData(boost::dynamic_pointer_cast<Parser::Int>(line));
        Parser::BytePtr byteData(boost::dynamic_pointer_cast<Parser::Byte>(line));
        Parser::InstructionPtr instruction(boost::dynamic_pointer_cast<Parser::Instruction>(line));

        if (byteData) {
            // This is byte data
            m_block[curAddr] = byteData->value;
            curAddr++;
        } else if (intData) {
            *((int*)&m_block[curAddr]) = intData->value;
            curAddr += sizeof(intData->value);
        } else if (instruction) {
            // This is an actual "machine" instruction
            // First, let's record the address of the first instruction (relative to the "machine")
            if (!start)
                start = curAddr;
            ParamType type = PT_DEFAULT;
            instructionBlock block;
            block.value = 0;
            if (instruction->args.size() >= 1) {
                char reg(getRegister(instruction->args[0]));
                if (reg >= 0) {
                    block.uint8_param = (unsigned char)reg;
                    type = PT_REG;
                } else if (labelAddr.find(instruction->args[0]) != labelAddr.end()) {
                    block.uint16_param2 = labelAddr[instruction->args[0]];
                    type = PT_ADDR;
                } else {
                    try {
                        block.uint16_param2 = boost::lexical_cast<unsigned short>(instruction->args[0]);
                        type = PT_IMMEDIATE;
                    } catch (...) { // boost::bad_lexical_cast
                        std::string msg = "Invalid command; unrecognized argument 1 (" + instruction->args[0] + ")on command " + instruction->name;
                        msg += " on line " + boost::lexical_cast<std::string>(byteToLineMap[curAddr]);
                        throw AssemblerException(msg);
                    }
                }
            }
            if (instruction->args.size() >= 2 && !block.uint16_param2) {
                char reg(getRegister(instruction->args[1]));
                if (reg >= 0) {
                    block.uint8_param2 = (unsigned char)reg;
                    type = PT_REGREG;
                } else if (labelAddr.find(instruction->args[1]) != labelAddr.end()) {
                    block.uint16_param2 = labelAddr[instruction->args[1]];
                    type = PT_REGADDR;
                } else {
                    try {
                        block.uint16_param2 = static_cast<int16_t>(boost::lexical_cast<short>(instruction->args[1]));
                        type = PT_REGIMMEDIATE;
                    } catch (...) { // boost::bad_lexical_cast
                        std::string msg = "Invalid command; unrecognized argument 2 (" + instruction->args[1] + ") on command " + instruction->name;
                        msg += " on line " + boost::lexical_cast<std::string>(byteToLineMap[curAddr]);
                        throw AssemblerException(msg);
                    }
                }
            }
            if (instruction->args.size() > 2 && !instruction->args[2].empty()) {
                throw AssemblerException("Too many arguments on command " + instruction->name + " on line " + boost::lexical_cast<std::string>(byteToLineMap[curAddr]));
            }
            block.instruction = m_config->strToBinary(instruction->name, type);
            *((int*)&m_block[curAddr]) = block.value;
            curAddr += sizeof(block.value);
        } else {
            throw AssemblerException("Invalid LinePtr found at " + boost::lexical_cast<std::string>(curAddr));
        }
    }

    return start;
}

Assembler::~Assembler(void)
{
}
