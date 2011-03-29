/*
 * =====================================================================================
 *
 *       Filename:  TCodeWriter.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  28.03.2011 16:40:41
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *        Company:  
 *
 * =====================================================================================
 */

#define DEBUG

#include "TCodeWriter.h"
#include "../Parser.h"
#include <iostream>

#include "../VMConfig.cpp"
#include "../Parser.cpp"


template<class F>
struct method_wrapper_NOSTR
{
    typedef void result_type;
    F f;
    method_wrapper_NOSTR(F f) : f(f) {}
    void operator()(TCodeWriter* writer, const Parser::InstructionPtr& inst)
    {
        return (writer->*f)();
    }
};
TWHandleFunctor
make_method(TCodeWriter* instance, void (TCodeWriter::*function)())
{
    TWHandleFunctor def = boost::bind(method_wrapper_NOSTR<void (TCodeWriter::*)()>(function), instance, _1);
    return def;
}
template<class F>
struct method_wrapper_ONESTR
{
    typedef void result_type;
    F f;
    method_wrapper_ONESTR(F f) : f(f) {}
    void operator()(TCodeWriter* writer, const Parser::InstructionPtr& inst)
    {
        return (writer->*f)(inst->args[0]);
    }
};
TWHandleFunctor
make_method(TCodeWriter* instance, void (TCodeWriter::*function)(const std::string&))
{
    TWHandleFunctor def = boost::bind(method_wrapper_ONESTR<void (TCodeWriter::*)(const std::string&)>(function), instance, _1);
    return def;
}
template<class F>
struct method_wrapper_TWOSTR
{
    typedef void result_type;
    F f;
    method_wrapper_TWOSTR(F f) : f(f) {}
    void operator()(TCodeWriter* writer, const Parser::InstructionPtr& inst)
    {
        return (writer->*f)(inst->args[0], inst->args[1]);
    }
};
TWHandleFunctor
make_method(TCodeWriter* instance, void (TCodeWriter::*function)(const std::string&, const std::string&))
{
    TWHandleFunctor def = boost::bind(method_wrapper_TWOSTR<void (TCodeWriter::*)(const std::string&, const std::string&)>(function), instance, _1);
    return def;
}
template<class F>
struct method_wrapper_THREESTR
{
    typedef void result_type;
    F f;
    method_wrapper_THREESTR(F f) : f(f) {}
    void operator()(TCodeWriter* writer, const Parser::InstructionPtr& inst)
    {
        return (writer->*f)(inst->args[0], inst->args[1], inst->args[2]);
    }
};
TWHandleFunctor
make_method(TCodeWriter* instance, void (TCodeWriter::*function)(const std::string&, const std::string&, const std::string&))
{
    TWHandleFunctor def = boost::bind(method_wrapper_THREESTR<void (TCodeWriter::*)(const std::string&, const std::string&, const std::string&)>(function), instance, _1);
    return def;
}

ICodeVMConfig::ICodeVMConfig()
{
    registerInstruction("FRAME",    0x01);
    registerInstruction("CALL",     0x02);
    registerInstruction("TRP",      0x03);
    registerInstruction("FUNC",     0x04);
    registerInstruction("MOVE",     0x05);
    registerInstruction("NEWI",     0x06);
    registerInstruction("WRITE",    0x07);
    registerInstruction("READ",     0x08);
    registerInstruction("PUSH",     0x09);
    registerInstruction("POP",      0x0A);
    registerInstruction("NOOP",     0x0B);
    registerInstruction("JMP",      0x0C);
    registerInstruction("RETURN",   0x0D);
    registerInstruction("RTN",      0x0E);

    registerInstruction("ADD",      0x0F);
    registerInstruction("SUB",      0x10);
    registerInstruction("MULT",     0x11);
    registerInstruction("DIV",      0x12);

    registerInstruction("EQ",       0x13);
    registerInstruction("NE",       0x14);
    registerInstruction("BF",       0x15);
    registerInstruction("BT",       0x16);
}

TCodeWriter::TCodeWriter(const std::string& icodeFile, const std::string& outFile)
    : m_inFile(icodeFile), m_outFile(outFile.c_str()),
    m_config(boost::make_shared<ICodeVMConfig>()), m_parser(icodeFile, m_config) {

    registerTWHandler("NOOP", make_method(this, &TCodeWriter::NOOP));
    registerTWHandler("JMP", make_method(this, &TCodeWriter::JMP));
    //registerTWHandler("FRAME", make_method(this, &TCodeWriter::FRAME));
    //registerTWHandler("CALL", make_method(this, &TCodeWriter::CALL));
    //registerTWHandler("TRP", make_method(this, &TCodeWriter::TRP));
    //registerTWHandler("FUNC", make_method(this, &TCodeWriter::FUNC));
    //registerTWHandler("MOVE", make_method(this, &TCodeWriter::MOVE));
    //registerTWHandler("NEWI", make_method(this, &TCodeWriter::NEWI));
    //registerTWHandler("WRITE", make_method(this, &TCodeWriter::WRITE));
    //registerTWHandler("READ", make_method(this, &TCodeWriter::READ));
    //registerTWHandler("PUSH", make_method(this, &TCodeWriter::PUSH));
    //registerTWHandler("POP", make_method(this, &TCodeWriter::POP));
    //registerTWHandler("RETURN", make_method(this, &TCodeWriter::RETURN));
    //registerTWHandler("RTN", make_method(this, &TCodeWriter::RTN));

    //registerTWHandler("ADD", make_method(this, &TCodeWriter::ADD));
    //registerTWHandler("SUB", make_method(this, &TCodeWriter::SUB));
    //registerTWHandler("MULT", make_method(this, &TCodeWriter::MULT));
    //registerTWHandler("DIV", make_method(this, &TCodeWriter::DIV));

    //registerTWHandler("EQ", make_method(this, &TCodeWriter::EQ));
    //registerTWHandler("NE", make_method(this, &TCodeWriter::NE));
    //registerTWHandler("BF", make_method(this, &TCodeWriter::BF));
    //registerTWHandler("BT", make_method(this, &TCodeWriter::BT));
}

void TCodeWriter::start() {
    Parser::LinePtr curLine;
    std::map<std::string, unsigned short> labelAddr;
    std::deque<Parser::LinePtr> lines;
#ifdef DEBUG
    std::cout << "Analyzing code..." << std::endl;
#endif
    while (curLine = m_parser.getNextLine()) {
        std::string lbl(curLine->label);
        lines.push_back(curLine);

        Parser::InstructionPtr inst(boost::dynamic_pointer_cast<Parser::Instruction>(curLine));
        Parser::CommentPtr comment(boost::dynamic_pointer_cast<Parser::Comment>(curLine));
        if (inst) {
            m_outFile << "; [" << inst->name;
            for (std::vector<std::string>::iterator it(inst->args.begin()); it != inst->args.end(); ++it) {
                m_outFile << " " << (*it);
            }
            m_outFile << "]";
            if (!inst->label.empty()) {
                m_outFile << "(" << inst->label.c_str() << ")";
                Label(inst->label);
            }
            
            m_outFile << std::endl;
            handleInstruction(inst);
        } else if (comment) {
            m_outFile << "; (KXI) " << comment->line << std::endl;
        } else {
            assert(false);
        }
    }
}

void TCodeWriter::Write( const std::string& instruction )
{
    Write(instruction, std::string(), std::string(), std::string());
}

void TCodeWriter::Write( const std::string& instruction, const std::string& op1 )
{
    Write(instruction, op1, std::string(), std::string());
}

void TCodeWriter::Write( const std::string& instruction, const std::string& op1, const std::string& op2 )
{
    Write(instruction, op1, op2, std::string());
}

void TCodeWriter::Write( const std::string& instruction, const std::string& op1, const std::string& op2, const std::string& op3 )
{
    std::string label(nextLabel);
    if (nextLineNo >= 0) {
        m_outFile << "                ; Line " << nextLineNo << std::endl;
    }
    do {
        label += " ";
    } while (label.size() < 16);

    m_outFile << label.c_str() << instruction.c_str();
    if (!op1.empty()) m_outFile << " " << op1.c_str();
    if (!op2.empty()) m_outFile << ", " << op2.c_str();
    if (!op3.empty()) m_outFile << ", " << op3.c_str();
    m_outFile << std::endl;
    nextLabel = "";
    nextLineNo = -1;
}

void TCodeWriter::Blank()
{
    m_outFile << std::endl;
}

void TCodeWriter::Comment( const std::string& comment, const bool indent/* = true*/ )
{
    if (indent) {
        m_outFile << "                ";
    }
    m_outFile << "; " << comment.c_str() << std::endl;
}

void TCodeWriter::handleInstruction(const Parser::InstructionPtr& inst)
{
    std::string name(inst->name);
    if (m_handlerMap.find(name) != m_handlerMap.end()) {
        m_handlerMap[name](inst);
    } else {
        Comment("TODO: Implement " + name + " target code generation");
    }
}

void TCodeWriter::NOOP()
{
    Write("ADI", "R0", "0");
}
void TCodeWriter::JMP(const std::string& label)
{
    Write("JMP", label);
}

void FRAME(const std::string& param1, const std::string& param2)
{
}
void CALL(const std::string& param1)
{
}
void TRP(const std::string& param1)
{
}
void FUNC(const std::string& param1)
{
}
void MOVE(const std::string& param1, const std::string& param2)
{
}
void NEWI(const std::string& param1, const std::string& param2)
{
}
void WRITE(const std::string& param1)
{
}
void READ(const std::string& param1)
{
}
void PUSH(const std::string& param1)
{
}
void POP(const std::string& param1)
{
}
void RETURN(const std::string& param1)
{
}
void RTN()
{
}

void ADD(const std::string& param1, const std::string& param2, const std::string& param3)
{
}
void SUB(const std::string& param1, const std::string& param2, const std::string& param3)
{
}
void MULT(const std::string& param1, const std::string& param2, const std::string& param3)
{
}
void DIV(const std::string& param1, const std::string& param2, const std::string& param3)
{
}

void EQ(const std::string& param1, const std::string& param2, const std::string& param3)
{
}
void NE(const std::string& param1, const std::string& param2, const std::string& param3)
{
}
void BF(const std::string& param1, const std::string& param2, const std::string& param3)
{
}
void BT(const std::string& param1, const std::string& param2, const std::string& param3)
{
}
