#include "Assembler.h"
#include <boost/make_shared.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include "VMConfig.h"

char Assembler::getRegister(std::string name)
{
    if (name[0] != 'R' || name.size() > 3)
        return -1;
    try {
        return (char)boost::lexical_cast<int>(name.substr(1, name.size() - 1));
    } catch (...) {
        return -1;
    }
}

Assembler::Assembler(std::string filename) : m_config(boost::make_shared<VMConfig>()), m_parser(filename, m_config), m_block(new unsigned int[1024])
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
        std::string lbl(curLine->label);
        if (!curLine->label.empty()) {
            if (labelAddr.find(curLine->label) != labelAddr.end()) {
                std::string msg("Duplicate label: ");
                msg += curLine->label;
                msg += " on line ";
                msg += m_parser.getLineNumber();
                throw AssemblerException(msg);
            }
            labelAddr[curLine->label] = curAddr;
        }
        lines.push_back(curLine);
        byteToLineMap[curAddr] = m_parser.getLineNumber();
        curAddr++;
    }
#ifdef DEBUG
    std::cout << "Processed: " << m_parser.getLineNumber() << " lines. " << labelAddr.size() << " labels, "
        << lines.size() * 4 << " bytes used." << std::endl;
    std::cout << "Building binary block..." << std::endl;
#endif

    curAddr = 0;
    while (lines.size()) {
        Parser::LinePtr line(lines.front());
        lines.pop_front();
        Parser::DataPtr data(boost::dynamic_pointer_cast<Parser::Data>(line));
        Parser::InstructionPtr instruction(boost::dynamic_pointer_cast<Parser::Instruction>(line));

        if (data) {
            // This is data
            m_block[curAddr] = data->value;

        } else if (instruction) {
            // This is an actual "machine" instruction
            // First, let's record the address of the first instruction (relative to the "machine")
            if (!start)
                start = curAddr;

            instructionBlock block;
            block.value = 0;
            block.instruction = m_config->strToBinary(instruction->name);
            if (instruction->args.size() >= 1) {
                char reg(getRegister(instruction->args[0]));
                if (reg >= 0) {
                    block.uint8_param = (unsigned char)reg;
                } else if (labelAddr.find(instruction->args[0]) != labelAddr.end()) {
                    block.uint16_param2 = labelAddr[instruction->args[0]];
                } else {
                    try {
                        block.uint16_param2 = boost::lexical_cast<unsigned short>(instruction->args[0]);
                    } catch (...) { // boost::bad_lexical_cast
                        throw AssemblerException("Invalid command; unrecognized argument on command " + instruction->name);
                    }
                }
            }
            if (instruction->args.size() >= 2 && !block.uint16_param2) {
                char reg(getRegister(instruction->args[1]));
                if (reg >= 0) {
                    block.uint8_param2 = (unsigned char)reg;
                } else if (labelAddr.find(instruction->args[1]) != labelAddr.end()) {
                    block.uint16_param2 = labelAddr[instruction->args[1]];
                } else {
                    try {
                        block.uint16_param2 = boost::lexical_cast<unsigned short>(instruction->args[1]);
                    } catch (...) { // boost::bad_lexical_cast
                        throw AssemblerException("Invalid command; unrecognized argument on command " + instruction->name);
                    }
                }
            }
            if (instruction->args.size() == 3 && !block.uint16_param2) {
                char reg(getRegister(instruction->args[2]));
                if (reg >= 0) {
                    block.uint8_param2 = (unsigned char)reg;
                } else {
                    try {
                        block.uint8_param3 = (unsigned char)boost::lexical_cast<unsigned short>(instruction->args[2]);
                    } catch (...) { // boost::bad_lexical_cast
                        throw AssemblerException("Invalid command; unrecognized argument on command " + instruction->name);
                    }
                }
            }
            if (instruction->args.size() > 3) {
                throw AssemblerException("Too many arguments on command " + instruction->name);
            }
            m_block[curAddr] = block.value;
        } else {
            throw AssemblerException("Invalid LinePtr found at " + curAddr);
        }
        curAddr++;
    }

    return start;
}



Assembler::~Assembler(void)
{
}
