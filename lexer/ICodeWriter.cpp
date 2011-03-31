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
        getStream() << "                ; Line " << nextLineNo << std::endl;
    }
    do {
        label += " ";
    } while (label.size() < 16);

    getStream() << label.c_str() << instruction.c_str();
    if (!op1.empty()) getStream() << " " << op1.c_str();
    if (!op2.empty()) getStream() << ", " << op2.c_str();
    if (!op3.empty()) getStream() << ", " << op3.c_str();
    getStream() << std::endl;
    nextLabel = "";
    nextLineNo = -1;
}

void ICodeWriter::Blank()
{
    getStream() << std::endl;
}

void ICodeWriter::Comment( const std::string& comment, const bool indent/* = true*/ )
{
    if (indent) {
        getStream() << "                ";
    }
    getStream() << "; " << comment.c_str() << std::endl;
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
    else if (op == "||") inst = "OR";
    else {
        assert(false); //panic!
    }
    Write(inst, tempId, op1, op2);
}

void ICodeWriter::beginSection(const std::string& name)
{
    curSection = name;
    if (m_sections.find(name) == m_sections.end()) {
        m_sections[name] = std::string();
    }
}
void ICodeWriter::endSection()
{
	std::string prevText = m_sections[curSection];
	std::string newText = buffer.str();
    m_sections[curSection] = prevText + newText;
    curSection = "";
    buffer.str("");
}
void ICodeWriter::writeSection(const std::string& name)
{
    getStream() << m_sections[name];
    m_sections.erase(m_sections.find(name));
}
