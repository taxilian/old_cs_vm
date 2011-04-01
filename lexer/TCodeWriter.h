
#ifndef H_TcodeWriter
#define H_TcodeWriter

#include <boost/make_shared.hpp>
#include <string>
#include <fstream>
#include "../VMConfig.h"
#include "../Parser.h"
#include "SymbolEntry.h"

class ICodeVMConfig : public VMConfig
{
public:
    ICodeVMConfig();
};

typedef boost::function<void (const Parser::InstructionPtr&)> TWHandleFunctor;

class TCodeWriter
{
public:
    static const int INST_SIZE = 4;
    TCodeWriter(const std::string& icodeFile, const std::string& outFile);

    void start();

    void LineNumber(const int lineNo) { nextLineNo = -1; }
    void Label(const std::string& label) { nextLabel = label; }
    void Write(const std::string& instruction, const std::string& op1 );
    void Write(const std::string& instruction, const std::string& op1, const std::string& op2, const std::string& comment = "");
    void Write(const std::string& instruction);

    void Blank();
    void Comment(const std::string& comment, const bool indent = true);

    void setSymbolIdMap(const std::map<std::string, SymbolEntryPtr>& smap) { symbol_id_map = smap; }
    void setSymbolNameMap(const std::map<std::string, SymbolEntryPtr>& smap) { symbol_name_map = smap; }
protected:
    // Helper things
    void LoadToReg(const std::string& reg, const std::string& src);
    void StoreFromReg(const std::string& reg, const std::string& src);
    int GetLocalOffset(const std::string& id);
public:

    void handleInstruction(const Parser::InstructionPtr& inst);
    void NOOP();
    void JMP(const std::string& label);
    void FRAME(const std::string& param1, const std::string& param2);
    void CALL(const std::string& param1);
    void TRP(const std::string& param1);
    void FUNC(const std::string& param1);
    void MOVE(const std::string& param1, const std::string& param2);
    void NEW(const std::string& param1, const std::string& param2);
    void WRITE(const std::string& param1);
    void READ(const std::string& param1);
    void PUSH(const std::string& param1);
    void PEEK(const std::string& param1);
    void POP(const std::string& param1);
    void RETURN(const std::string& param1);
    void RTN();
    void REF(const std::string& param1, const std::string& param2, const std::string& param3);

    void AND(const std::string& param1, const std::string& param2, const std::string& param3);
    void OR(const std::string& param1, const std::string& param2, const std::string& param3);
    void ADD(const std::string& param1, const std::string& param2, const std::string& param3);
    void SUB(const std::string& param1, const std::string& param2, const std::string& param3);
    void MULT(const std::string& param1, const std::string& param2, const std::string& param3);
    void DIV(const std::string& param1, const std::string& param2, const std::string& param3);

    void LE(const std::string& param1, const std::string& param2, const std::string& param3);
    void GE(const std::string& param1, const std::string& param2, const std::string& param3);
    void LT(const std::string& param1, const std::string& param2, const std::string& param3);
    void GT(const std::string& param1, const std::string& param2, const std::string& param3);
    void EQ(const std::string& param1, const std::string& param2, const std::string& param3);
    void NE(const std::string& param1, const std::string& param2, const std::string& param3);
    void BF(const std::string& param1, const std::string& param2);
    void BT(const std::string& param1, const std::string& param2);

    enum GlobalDataType {
        GDT_Char,
        GDT_Int
    };
protected:
    void registerTWHandler(const std::string& kw, const TWHandleFunctor& func) {
        m_handlerMap[kw] = func;
    }
    int getTypeSize( const std::string& type, bool nested = false );
    
private:
    std::string m_inFile;
    std::ofstream m_outFile;
    VMConfigPtr m_config;
    Parser m_parser;
    std::map<std::string, TWHandleFunctor> m_handlerMap;
    std::string nextLabel;
    int nextLineNo;
    struct GlobalData {
        GlobalDataType type;
        std::string value;
        std::string ID;
    };
    int lastId;
    std::map<std::string, GlobalData> global_map;
    std::map<std::string, GlobalData> global_id_map;
    std::map<std::string, SymbolEntryPtr> symbol_id_map;
    std::map<std::string, SymbolEntryPtr> symbol_name_map;

    std::map<std::string, int> offsetMap;
};

#endif
