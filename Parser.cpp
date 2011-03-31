#include "Parser.h"
#include <boost/algorithm/string.hpp>
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

std::vector<std::string> Parser::split(const std::string &str, const char *tokens)
{
    std::vector<std::string> outList;
    
    boost::tokenizer<my_escaped_list_separator<char> > escTokenizer(str, my_escaped_list_separator<char>("\\", ", \t\r\n", "\"'", ";"));
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

        line = std::string(buffer);
        boost::algorithm::erase_all(line, "\r");
        size_t begPos(line.find_first_not_of("\t "));
        if (begPos != std::string::npos && line[begPos] == ';') {
            // This line is a comment!
            CommentPtr c(boost::make_shared<Comment>());
            c->line = line;
            m_queue.push_back(c);
            return;
        }
        std::vector<std::string> tokens(split(line, ", \r\n()"));
        std::vector<std::string>::iterator it = tokens.begin();
        if (it != tokens.end() && it->empty())
            nextToken(it, tokens.end());

        try {
            if (it == tokens.end()) {
                continue;
            } else {
                std::string label;
                std::string instruct;
                if ((*it)[0] != '.' && !m_config->isValidInstruction(*it)) {
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
                    
                    if (instruct == "BYT") {
                        if (data[0] == '"' || data[0] == '\'') {
                            std::string chars = data.substr(1, data.size() - 2);
                            for (std::string::size_type i = 0; i < chars.size(); i++) {
                                BytePtr d(boost::make_shared<Byte>());
                                d->label = label;
                                label = "";
                                d->value = chars[i];
                                m_queue.push_back(d);
                            }
                        } else { 
                            BytePtr d(boost::make_shared<Byte>());
                            d->label = label;
                            d->value = boost::lexical_cast<int>(data);
                            m_queue.push_back(d);
                        }
                    } else if (instruct == "INT") {
                        IntPtr d(boost::make_shared<Int>());
                        d->label = label;
                        d->value = boost::lexical_cast<int>(data);
                        m_queue.push_back(d);
                    } else {
                        throw ParserException("Bad DATA type found");
                    }
                    // This is a data piece
                } else {
                    InstructionPtr ins(boost::make_shared<Instruction>());
                    ins->label = label;
                    ins->name = instruct;
                    while (++it != tokens.end()) {
                        ins->args.push_back(*it);
                    }
                    m_queue.push_back(ins);
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
