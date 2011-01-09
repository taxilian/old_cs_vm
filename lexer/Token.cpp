#include "Token.h"
#include <sstream>

std::string Token::toString()
{
    std::stringstream ss;
    switch(type) {
    case TT_ENDOFFILE:
        ss << "End of File.";
        return ss.str();
    case TT_ENDOFLINE:
        ss << "End of line.";
        return ss.str();
    case TT_GROUPOPEN:
        ss << "Grp Open: ";
        break;
    case TT_GROUPCLOSE:
        ss << "GRPCLOSE: ";
        break;
    case TT_NUMBER:
        ss << "Number: ";
        break;
    case TT_KEYWORD:
        ss << "Keyword/Identifier: ";
        break;
    case TT_STRING:
        ss << "string: ";
        break;
    case TT_CHAR:
        ss << "char: ";
        break;
    case TT_WHITESPACE:
        ss << "<Whitespace>";
        return ss.str();
    case TT_OPERATOR:
        ss << "Operator: ";
        break;
    default:
        ss << "UNKNOWN!!: ";
    }

    ss << text;
    return ss.str();
}
