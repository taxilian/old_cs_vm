#pragma once
#include <fstream>
#include <string>
#include <sstream>
#include <map>

class ICodeWriter
{
public:
    ICodeWriter(const std::string& fname);
    ~ICodeWriter(void);

    void LineNumber(const int lineNo) { nextLineNo = -1; }
    void Label(const std::string& label) { nextLabel = label; }
    void Write(const std::string& instruction, const std::string& op1);
    void Write(const std::string& instruction, const std::string& op1, const std::string& op2);
    void Write(const std::string& instruction, const std::string& op1, const std::string& op2, const std::string& op3);
    void Write(const std::string& instruction);

    void Blank();
    void Comment(const std::string& comment);
    void DoMath( std::string op, std::string tempId, std::string op1, std::string op2 );

    void beginSection(const std::string& name);
    void endSection();
    void writeSection(const std::string& name);

private:
    std::ostream& getStream() {
        if (curSection.empty())
			return file;
        else
			return buffer;
    }
private:
    std::ofstream file;
	std::ostringstream buffer;
    std::string fname;

    std::string curSection;
    std::map<std::string, std::string> m_sections;
    std::string nextLabel;
    int nextLineNo;
};

