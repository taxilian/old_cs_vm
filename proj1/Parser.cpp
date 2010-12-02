#include "Parser.h"
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include "escaped_list.h"

Parser::Parser(std::string filename, VMConfigPtr config) : m_file(filename.c_str()), end(false), m_config(config), m_lineNumber(0)
{
}

Parser::~Parser(void)
{
}

std::string Parser::sanitizeString(const std::string &str)
{
    std::string::size_type search, left, right;
    std::string sub(str);
    if ((search = str.find(";")) != std::string::npos) {
        sub = str.substr(0, search);
    }
    left = sub.find_first_not_of(" \t");
    right = sub.find_last_not_of(" \t");
    if (left != std::string::npos)
        return sub.substr(left, right-left+1);
    else
        return sub;
}

//
//
std::vector<std::string> Parser::split(const std::string &str, const char *tokens)
{
    std::vector<std::string> outList;
    
    boost::tokenizer<escaped_list_separator<char> > escTokenizer(str, escaped_list_separator<char>("\\", ", \t\r\n", "\"", ";"));
    BOOST_FOREACH(std::string s, escTokenizer)
        outList.push_back(s);
     
    return outList;
}

std::string nextToken(std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end)
{
    while (it != end && ++it != end && it->empty()) {
    }
    if (it == end)
        return std::string();
    else
        return *it;
}

void Parser::processFile()
{
    char buffer[256];
    while (m_file.getline(buffer, 256, '\n')) {
        std::string line;
        m_lineNumber++;

        line = std::string(buffer); //sanitizeString(std::string(buffer));
        std::vector<std::string> tokens(split(line, ", \r\n()"));
        std::vector<std::string>::iterator it = tokens.begin();
        if (it != tokens.end() && it->empty())
            nextToken(it, tokens.end());

        try {
            if (it == tokens.end()) {
                continue;
            } else if (tokens.size() == 1) {
                throw ParserException("Only found 1 token");
            } else {
                std::string label;
                std::string instruct;
                if (!m_config->isValidInstruction(*it)
                    && (*it)[0] != '.') {
                    label = *it;
                    nextToken(it, tokens.end());
                }
                instruct = *it;
                if (instruct[0] == '.') {
                    // This is a data line
                    instruct = instruct.substr(1, instruct.size() - 1);
                    std::string data = nextToken(it, tokens.end());
                    if (it+1 != tokens.end()) {
                        throw ParserException("Too many tokens on the line.");
                    }
                    
                    if (instruct == "BYT" && 
                        (data[0] == '"' || data[0] == '\'')) {
                        std::string chars = data.substr(1, data.size() - 2);
                        for (std::string::size_type i = 0; i < chars.size(); i++) {
                            DataPtr d(boost::make_shared<Data>());
                            d->label = label;
                            label = "";
                            d->value = (int)chars[i];
                            m_queue.push_back(boost::dynamic_pointer_cast<Line>(d));
                        }
                    } else if (instruct == "BYT" || instruct == "INT") {
                        DataPtr d(boost::make_shared<Data>());
                        d->label = label;
                        d->value = boost::lexical_cast<int>(data);
                        m_queue.push_back(boost::dynamic_pointer_cast<Line>(d));
                    } else {
                        throw std::exception("Bad DATA type found");
                    }
                    // This is a data piece
                } else {
                    InstructionPtr ins(boost::make_shared<Instruction>());;
                    ins->label = label;
                    ins->name = instruct;
                    while (++it != tokens.end()) {
                        ins->args.push_back(*it);
                    }
                    m_queue.push_back(boost::dynamic_pointer_cast<Line>(ins));
                }
            }
        } catch (std::exception &ex) {
            std::string msg(ex.what());
            msg += ": on line " + m_lineNumber;
            throw ParserException(msg);
        }
        if (m_queue.size() > 0)
            return;
    }
}

Parser::LinePtr Parser::getNextLine()
{
    if (!m_queue.size()) {
        this->processFile();
        if (!m_queue.size()) {
            // If there is nothing on the queue after running processFile,
            // we've reached the end of the file
            return LinePtr();
        }
    }
    LinePtr ret(m_queue.front());
    m_queue.pop_front();
    
    return ret;
}