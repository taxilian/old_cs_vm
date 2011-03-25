#include <string>
#include "ICodeWriter.h"
#include <assert.h>

ICodeWriter::ICodeWriter( const std::string& fname ) : fname(fname), file(fname.c_str())
{
}


ICodeWriter::~ICodeWriter(void)
{
    file.close();
}

void ICodeWriter::Write( const std::string& instruction )
{
    Write(instruction, std::string(), std::string(), std::string());
}

void ICodeWriter::Write( const std::string& instruction, const std::string& op1 )
{
    Write(instruction, op1, std::string(), std::string());
}

void ICodeWriter::Write( const std::string& instruction, const std::string& op1, const std::string& op2 )
{
    Write(instruction, op1, op2, std::string());
}

void ICodeWriter::Write( const std::string& instruction, const std::string& op1, const std::string& op2, const std::string& op3 )
{
    std::string label(nextLabel);
    if (nextLineNo >= 0) {
        file << "                ; Line " << nextLineNo << std::endl;
    }
    do {
        label += " ";
    } while (label.size() < 16);

    file << label.c_str() << instruction.c_str();
    if (!op1.empty()) file << " " << op1.c_str();
    if (!op2.empty()) file << ", " << op2.c_str();
    if (!op3.empty()) file << ", " << op3.c_str();
    file << std::endl;
    nextLabel = "";
    nextLineNo = -1;
}

void ICodeWriter::Blank()
{
    file << std::endl;
}

void ICodeWriter::Comment( const std::string& comment )
{
    file << "                ; " << comment.c_str() << std::endl;
}

void ICodeWriter::DoMath( std::string op, std::string tempId, std::string op1, std::string op2 )
{
    std::string inst;
    if (op == "+") inst = "ADD";
    else if (op == "-") inst = "SUB";
    else if (op == "*") inst = "MULT";
    else if (op == "/") inst = "DIV";
    else if (op == "==") inst = "EQ";
    else if (op == "!=") inst = "NE";
    else if (op == ">") inst = "GT";
    else if (op == "<") inst = "LT";
    else if (op == ">=") inst = "GE";
    else if (op == "<=") inst = "LE";
    else if (op == "&&") inst = "AND";
    else if (op == "||") inst = "AND";
    else {
        assert(false); //panic!
    }
    Write(inst, tempId, op1, op2);
}
