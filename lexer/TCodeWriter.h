
#ifndef H_TcodeWriter
#define H_TcodeWriter

#include <boost/make_shared.hpp>
#include <string>
#include <fstream>
#include "../VMConfig.h"
#include "../Parser.h"

class ICodeVMConfig : public VMConfig
{
public:
    ICodeVMConfig();
};

typedef boost::function<void (const Parser::InstructionPtr&)> TWHandleFunctor;

class TCodeWriter
{
public:
    TCodeWriter(const std::string& icodeFile, const std::string& outFile);

    void start();

    void LineNumber(const int lineNo) { nextLineNo = -1; }
    void Label(const std::string& label) { nextLabel = label; }
    void Write(const std::string& instruction, const std::string& op1);
    void Write(const std::string& instruction, const std::string& op1, const std::string& op2);
    void Write(const std::string& instruction, const std::string& op1, const std::string& op2, const std::string& op3);
    void Write(const std::string& instruction);

    void Blank();
    void Comment(const std::string& comment, const bool indent = true);

public:
    void handleInstruction(const Parser::InstructionPtr& inst);
    void NOOP();
    void JMP(const std::string& label);
    void FRAME(const std::string& param1, const std::string& param2);
    void CALL(const std::string& param1);
    void TRP(const std::string& param1);
    void FUNC(const std::string& param1);
    void MOVE(const std::string& param1, const std::string& param2);
    void NEWI(const std::string& param1, const std::string& param2);
    void WRITE(const std::string& param1);
    void READ(const std::string& param1);
    void PUSH(const std::string& param1);
    void POP(const std::string& param1);
    void RETURN(const std::string& param1);
    void RTN();

    void ADD(const std::string& param1, const std::string& param2, const std::string& param3);
    void SUB(const std::string& param1, const std::string& param2, const std::string& param3);
    void MULT(const std::string& param1, const std::string& param2, const std::string& param3);
    void DIV(const std::string& param1, const std::string& param2, const std::string& param3);

    void EQ(const std::string& param1, const std::string& param2, const std::string& param3);
    void NE(const std::string& param1, const std::string& param2, const std::string& param3);
    void BF(const std::string& param1, const std::string& param2, const std::string& param3);
    void BT(const std::string& param1, const std::string& param2, const std::string& param3);

protected:
    void registerTWHandler(const std::string& kw, const TWHandleFunctor& func) {
        m_handlerMap[kw] = func;
    }
    
private:
    std::string m_inFile;
    std::ofstream m_outFile;
    VMConfigPtr m_config;
    Parser m_parser;
    std::map<std::string, TWHandleFunctor> m_handlerMap;
    std::string nextLabel;
    int nextLineNo;
};

#endif
