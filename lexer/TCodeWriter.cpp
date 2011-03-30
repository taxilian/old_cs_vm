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
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;

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
make_method(TCodeWriter* instance, void (TCodeWriter::*function)(const string&))
{
    TWHandleFunctor def = boost::bind(method_wrapper_ONESTR<void (TCodeWriter::*)(const string&)>(function), instance, _1);
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
make_method(TCodeWriter* instance, void (TCodeWriter::*function)(const string&, const string&))
{
    TWHandleFunctor def = boost::bind(method_wrapper_TWOSTR<void (TCodeWriter::*)(const string&, const string&)>(function), instance, _1);
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
make_method(TCodeWriter* instance, void (TCodeWriter::*function)(const string&, const string&, const string&))
{
    TWHandleFunctor def = boost::bind(method_wrapper_THREESTR<void (TCodeWriter::*)(const string&, const string&, const string&)>(function), instance, _1);
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

    registerInstruction("LT",       0x13);
    registerInstruction("GT",       0x14);
    registerInstruction("EQ",       0x15);
    registerInstruction("NE",       0x16);
    registerInstruction("BF",       0x17);
    registerInstruction("BT",       0x18);

    registerInstruction("REF",      0x19);
}

TCodeWriter::TCodeWriter(const string& icodeFile, const string& outFile)
    : m_inFile(icodeFile), m_outFile(outFile.c_str()),
    m_config(boost::make_shared<ICodeVMConfig>()), m_parser(icodeFile, m_config),
    lastId(5000) {

    registerTWHandler("NOOP", make_method(this, &TCodeWriter::NOOP));
    registerTWHandler("JMP", make_method(this, &TCodeWriter::JMP));
    registerTWHandler("FRAME", make_method(this, &TCodeWriter::FRAME));
    registerTWHandler("CALL", make_method(this, &TCodeWriter::CALL));
    registerTWHandler("TRP", make_method(this, &TCodeWriter::TRP));
    registerTWHandler("FUNC", make_method(this, &TCodeWriter::FUNC));
    registerTWHandler("MOVE", make_method(this, &TCodeWriter::MOVE));
    //registerTWHandler("NEWI", make_method(this, &TCodeWriter::NEWI));
    registerTWHandler("WRITE", make_method(this, &TCodeWriter::WRITE));
    registerTWHandler("READ", make_method(this, &TCodeWriter::READ));
    registerTWHandler("PUSH", make_method(this, &TCodeWriter::PUSH));
    registerTWHandler("POP", make_method(this, &TCodeWriter::POP));
    registerTWHandler("RETURN", make_method(this, &TCodeWriter::RETURN));
    registerTWHandler("RTN", make_method(this, &TCodeWriter::RTN));
    //registerTWHandler("REF", make_method(this, &TCodeWriter::REF));

    registerTWHandler("ADD", make_method(this, &TCodeWriter::ADD));
    registerTWHandler("SUB", make_method(this, &TCodeWriter::SUB));
    registerTWHandler("MULT", make_method(this, &TCodeWriter::MULT));
    registerTWHandler("DIV", make_method(this, &TCodeWriter::DIV));

    registerTWHandler("LT", make_method(this, &TCodeWriter::EQ));
    registerTWHandler("GT", make_method(this, &TCodeWriter::NE));
    registerTWHandler("EQ", make_method(this, &TCodeWriter::EQ));
    registerTWHandler("NE", make_method(this, &TCodeWriter::NE));
    registerTWHandler("BF", make_method(this, &TCodeWriter::BF));
    registerTWHandler("BT", make_method(this, &TCodeWriter::BT));
}

inline bool isGlobal(const string& param) {
    return param[0] == '+' || param[0] == '-' || (param[0] >= '0' && param[0] <= '9') || param[0] == '\'';
}

inline TCodeWriter::GlobalDataType getGDType(const string& param) {
    if (param[0] == '\'')
        return TCodeWriter::GDT_Char;
    else
        return TCodeWriter::GDT_Int;
}

inline string asString(int a) { return boost::lexical_cast<string>(a); }

void TCodeWriter::start() {
    Parser::LinePtr curLine;
    map<string, unsigned short> labelAddr;
    deque<Parser::LinePtr> lines;
#ifdef DEBUG
    cout << "Analyzing code..." << endl;
#endif
    while (curLine = m_parser.getNextLine()) {
        lines.push_back(curLine);

        Parser::InstructionPtr inst(boost::dynamic_pointer_cast<Parser::Instruction>(curLine));
        if (inst) {
            for (vector<string>::iterator it(inst->args.begin()); it != inst->args.end(); ++it) {
                if (isGlobal(*it)) {
                    if (global_map.find(*it) == global_map.end()) {
                        GlobalData d;
                        d.type = getGDType(*it);
                        d.value = *it;
                        boost::algorithm::replace_first(d.value, "\n", "\\n");
                        boost::algorithm::replace_first(d.value, "\r", "\\r");
                        boost::algorithm::replace_first(d.value, "\0", "\\0");
                        d.ID = string("G") + asString(lastId++);
                        global_map[*it] = d;
                        global_id_map[d.ID] = d;
                    }
                    *it = global_map[*it].ID;
                }
            }
        }
    }

    for (map<string, GlobalData>::iterator it(global_map.begin()); it != global_map.end(); ++it) {
        Label(it->second.ID);
        switch(it->second.type) {
            case GDT_Char:
                Write(".BYT", it->second.value);
                break;
            case GDT_Int:
                Write(".INT", it->second.value);
                break;
        }
    }

    while (lines.size()) {
        curLine = lines.front();
        lines.pop_front();
        string lbl(curLine->label);

        Parser::InstructionPtr inst(boost::dynamic_pointer_cast<Parser::Instruction>(curLine));
        Parser::CommentPtr comment(boost::dynamic_pointer_cast<Parser::Comment>(curLine));
        if (inst) {
            m_outFile << "; [" << inst->name;
            for (vector<string>::iterator it(inst->args.begin()); it != inst->args.end(); ++it) {
                m_outFile << " " << (*it);
            }
            m_outFile << "]";
            if (!inst->label.empty()) {
                m_outFile << "(" << inst->label.c_str() << ")";
                Label(inst->label);
            }
            
            m_outFile << endl;
            handleInstruction(inst);
        } else if (comment) {
            m_outFile << "; (KXI) " << comment->line << endl;
        } else {
            assert(false);
        }
    }
}

void TCodeWriter::Write( const string& instruction )
{
    Write(instruction, string(), string(), string());
}

void TCodeWriter::Write( const string& instruction, const string& op1 )
{
    Write(instruction, op1, string(), string());
}

void TCodeWriter::Write( const string& instruction, const string& op1, const string& op2 )
{
    Write(instruction, op1, op2, string());
}

void TCodeWriter::Write( const string& instruction, const string& op1, const string& op2, const string& op3 )
{
    string label(nextLabel);
    if (nextLineNo >= 0) {
        m_outFile << "                ; Line " << nextLineNo << endl;
    }
    do {
        label += " ";
    } while (label.size() < 16);

    m_outFile << label.c_str() << instruction.c_str();
    if (!op1.empty()) m_outFile << " " << op1.c_str();
    if (!op2.empty()) m_outFile << ", " << op2.c_str();
    if (!op3.empty()) m_outFile << ", " << op3.c_str();
    m_outFile << endl;
    nextLabel = "";
    nextLineNo = -1;
}

void TCodeWriter::Blank()
{
    m_outFile << endl;
}

void TCodeWriter::Comment( const string& comment, const bool indent/* = true*/ )
{
    if (indent) {
        m_outFile << "                ";
    }
    m_outFile << "; " << comment.c_str() << endl;
}

void TCodeWriter::handleInstruction(const Parser::InstructionPtr& inst)
{
    string name(inst->name);
    if (m_handlerMap.find(name) != m_handlerMap.end()) {
        m_handlerMap[name](inst);
    } else {
        Comment("TODO: Implement " + name + " target code generation");
    }
}

int TCodeWriter::GetLocalOffset(const string& id)
{
    assert(id[0] == 'T' || id[0] == 'L' || id[0] == 'P');
    if (offsetMap.find(id) != offsetMap.end())
        return offsetMap[id];
    
    SymbolEntryPtr sym1(symbol_id_map[id]);
    SymbolEntryPtr parent(symbol_name_map[sym1->scope]);
    MethodDataPtr mdata(boost::dynamic_pointer_cast<MethodData>(parent->data));
    assert(mdata);
    int i(0);
    for(vector<string>::iterator it(mdata->Parameters.begin()); it != mdata->Parameters.end(); ++it) {
        offsetMap[*it] = i++;
    }
    for(vector<string>::iterator it(mdata->vars.begin()); it != mdata->vars.end(); ++it) {
        offsetMap[*it] = i++;
    }
    return offsetMap[id];
}

void TCodeWriter::LoadToReg(const string& reg, const string& src)
{
    if (src[0] == 'G') {
        if (global_id_map[src].type == GDT_Char) {
            Write("LDB", reg, src);
        } else {
            Write("LDR", reg, src);
        }
    } else if (src[0] == 'T' || src[0] == 'L' || src[0] == 'P') {
        int offset = GetLocalOffset(src);
        Write("MOV", "R4", "FP");
        offset += 3;
        offset *= INST_SIZE;
        Write("ADI", "R4", asString(offset*-1));
        Write("LDR", reg, "R4");
    } else {
        Comment("TODO: LoadToReg " + src);
    }
}
void TCodeWriter::StoreFromReg(const string& reg, const string& dest)
{
    if (dest[0] == 'G') {
        if (global_id_map[dest].type == GDT_Char) {
            Write("STB", reg, dest);
        } else {
            Write("STR", reg, dest);
        }
    } else if (dest[0] == 'T' || dest[0] == 'L' || dest[0] == 'P') {
        int offset = GetLocalOffset(dest);
        Write("MOV", "R4", "FP");
        offset += 3;
        offset *= INST_SIZE;
        Write("ADI", "R4", asString(offset*-1));
        Write("STR", reg, "R4");
    } else {
        Comment("TODO: StoreFromReg " + dest);
    }
}

void TCodeWriter::NOOP()
{
    Write("ADI", "R0", "0");
}
void TCodeWriter::JMP(const string& label)
{
    Write("JMP", label);
}

void TCodeWriter::FRAME(const string& param1, const string& param2)
{
    // Set up the activation record for a new call:
    // (empty) <-- new SP
    // PFP
    // Return Address (new FP)
    Write("MOV", "R2", "SP");                        // R2 = Stack top
    Write("MOV", "R3", "SP");                        // R3 = Stack top
    Write("ADI", "R2", asString(INST_SIZE*-1));      // R2 = PFP location
    Write("STR", "FP", "R2");                        // Set PFP
    Write("ADI", "R2", asString(INST_SIZE*-1));      // New top of stack
    Write("MOV", "SP", "R2");                        // Stack PTR = R2
    Write("MOV", "FP", "R3");                        // Frame PTR = old Stack PTR

    PUSH(param2);
}
void TCodeWriter::CALL(const string& param1)
{
    Comment("Calling function " + param1);
    Write("MOV", "R1", "PC");                        // Get addr of next instruction
    Write("ADI", "R1", asString(INST_SIZE*3));       // Add 3 instructions from this one
    Write("STR", "R1", "FP");                        // Set return address
    Write("JMP", param1);
}
void TCodeWriter::TRP(const string& param1)
{
    Write("TRP", param1);
}
void TCodeWriter::FUNC(const string& param1)
{
    SymbolEntryPtr func(symbol_id_map[param1]);
    MethodDataPtr mdata(boost::dynamic_pointer_cast<MethodData>(func->data));
    assert(mdata);
    int i(0);
    for(vector<string>::iterator it(mdata->Parameters.begin()); it != mdata->Parameters.end(); ++it) {
        i++;
    }
    for(vector<string>::iterator it(mdata->vars.begin()); it != mdata->vars.end(); ++it) {
        i++;
    }
    i *= INST_SIZE;
    Comment("Allocate space on the stack for local variables");
    Write("ADI", "SP", asString(i*-1));
}
void TCodeWriter::MOVE(const string& param1, const string& param2)
{
    Comment("Load the value into R2");
    LoadToReg("R2", param2);

    Comment("Store the value into the destination");
    StoreFromReg("R2", param1);
}
void TCodeWriter::NEWI(const string& param1, const string& param2)
{
}
void TCodeWriter::WRITE(const string& param1)
{
    char type=0;
    if (param1[0] == 'G') {
        type = global_id_map[param1].type == GDT_Char ? 'c' : 'i';
    } else {
        std::string ct(boost::dynamic_pointer_cast<TypeData>(symbol_id_map[param1]->data)->type);
        type = ct == "char" ? 'c' : 'i';
    }
    if (type == 'c') {
        Comment("Loading char to print from " + param1);
        LoadToReg("R8", param1);
        Write("TRP", "3");
    } else {
        Comment("Loading int to print from " + param1);
        LoadToReg("R0", param1);
        Write("TRP", "1");
    }
}
void TCodeWriter::READ(const string& param1)
{
    std::string type(boost::dynamic_pointer_cast<TypeData>(symbol_id_map[param1]->data)->type);
    if (type == "char") {
        Write("TRP", "4");
        Comment("Saving char to " + param1);
        LoadToReg("R8", param1);
    } else {
        Write("TRP", "2");
        Comment("Saving int to " + param1);
        LoadToReg("R0", param1);
    }
}
void TCodeWriter::PUSH(const string& param1)
{
    if (param1[0] == 'R') { // If it's a register
        Write("STR", "SP", param1);
        Write("ADI", "SP", asString(INST_SIZE*-1));
    } else {
        Comment("Loading value to PUSH from " + param1);
        LoadToReg("R1", param1);
        PUSH("R1");
    }
}
void TCodeWriter::POP(const string& param1)
{
    if (param1[0] == 'R') { // If it's a register
        Write("ADI", "SP", asString(INST_SIZE));
        Write("LDR", param1, "SP");
    } else {
        POP("R1");
        Comment("Storing POPed value to " + param1);
        StoreFromReg("R1", param1);
    }
}
void TCodeWriter::RETURN(const string& param1)
{
    Comment("Returning " + param1);
    LoadToReg("R1", param1);
    Write("LDR", "R0", "FP");
    Write("STR", "R1", "FP");
    Write("MOV", "R5", "FP");
    Write("ADI", "R5", asString(INST_SIZE * -1));
    Write("MOV", "SP", "FP");
    Write("LDR", "FP", "R5");
    Write("JMR", "R0");
}
void TCodeWriter::RTN()
{
    Write("LDR", "R0", "FP");
    //Write("STR", "R1", "FP");
    Write("MOV", "R5", "FP");
    Write("ADI", "R5", asString(INST_SIZE * -1));
    Write("MOV", "SP", "FP");
    Write("LDR", "FP", "R5");
    Write("JMR", "R0");
}

void TCodeWriter::ADD(const string& param1, const string& param2, const string& param3)
{
    LoadToReg("R1", param2);
    LoadToReg("R2", param3);
    Write("ADD", "R1", "R2");
    StoreFromReg("R1", param1);
}
void TCodeWriter::SUB(const string& param1, const string& param2, const string& param3)
{
    LoadToReg("R1", param2);
    LoadToReg("R2", param3);
    Write("SUB", "R1", "R2");
    StoreFromReg("R1", param1);
}
void TCodeWriter::MULT(const string& param1, const string& param2, const string& param3)
{
    LoadToReg("R1", param2);
    LoadToReg("R2", param3);
    Write("MULT", "R1", "R2");
    StoreFromReg("R1", param1);
}
void TCodeWriter::DIV(const string& param1, const string& param2, const string& param3)
{
    LoadToReg("R1", param2);
    LoadToReg("R2", param3);
    Write("DIV", "R1", "R2");
    StoreFromReg("R1", param1);
}

void TCodeWriter::LT(const string& param1, const string& param2, const string& param3)
{
    LoadToReg("R1", param2);
    LoadToReg("R2", param2);
    Write("CMP", "R1", "R2");
    int cur(lastId++);
    std::string ltTag("LT_" + asString(cur));
    std::string nltTag("NLT_" + asString(cur));
    std::string doneTag("D_" + asString(cur));
    Write("BLT", ltTag);
    Write("JMP", nltTag);
    Label(ltTag);
    // Set R1 to 1 (TRUE) if LT
    Write("SUB", "R1", "R1");
    Write("ADI", "R1", "1");
    Write("JMP", doneTag);
    Label(nltTag);
    Write("SUB", "R1", "R1");
    Label(doneTag);
    StoreFromReg("R1", param1);

}
void TCodeWriter::GT(const string& param1, const string& param2, const string& param3)
{
    LoadToReg("R1", param2);
    LoadToReg("R2", param2);
    Write("CMP", "R1", "R2");
    int cur(lastId++);
    std::string gtTag("LT_" + asString(cur));
    std::string ngtTag("NLT_" + asString(cur));
    std::string doneTag("D_" + asString(cur));
    Write("BGT", gtTag);
    Write("JMP", ngtTag);
    Label(gtTag);
    // Set R1 to 1 (TRUE) if LT
    Write("SUB", "R1", "R1");
    Write("ADI", "R1", "1");
    Write("JMP", doneTag);
    Label(ngtTag);
    Write("SUB", "R1", "R1");
    Label(doneTag);
    StoreFromReg("R1", param1);
}
void TCodeWriter::EQ(const string& param1, const string& param2, const string& param3)
{
    LoadToReg("R1", param2);
    LoadToReg("R2", param2);
    Write("CMP", "R1", "R2");
    int cur(lastId++);
    std::string eqTag("LT_" + asString(cur));
    std::string neqTag("NLT_" + asString(cur));
    std::string doneTag("D_" + asString(cur));
    Write("BRZ", eqTag);
    Write("JMP", neqTag);
    Label(eqTag);
    // Set R1 to 1 (TRUE) if LT
    Write("SUB", "R1", "R1");
    Write("ADI", "R1", "1");
    Write("JMP", doneTag);
    Label(neqTag);
    Write("SUB", "R1", "R1");
    Label(doneTag);
    StoreFromReg("R1", param1);
}
void TCodeWriter::NE(const string& param1, const string& param2, const string& param3)
{
    LoadToReg("R1", param2);
    LoadToReg("R2", param2);
    Write("CMP", "R1", "R2");
    int cur(lastId++);
    std::string neTag("LT_" + asString(cur));
    std::string nneTag("NLT_" + asString(cur));
    std::string doneTag("D_" + asString(cur));
    Write("BNZ", neTag);
    Write("JMP", nneTag);
    Label(neTag);
    // Set R1 to 1 (TRUE) if LT
    Write("SUB", "R1", "R1");
    Write("ADI", "R1", "1");
    Write("JMP", doneTag);
    Label(nneTag);
    Write("SUB", "R1", "R1");
    Label(doneTag);
    StoreFromReg("R1", param1);
}
void TCodeWriter::BF(const string& param1, const string& param2)
{
    LoadToReg("R1", param1);
    Write("BRZ", param2);
}
void TCodeWriter::BT(const string& param1, const string& param2)
{
    LoadToReg("R1", param1);
    Write("BNZ", param2);
}
