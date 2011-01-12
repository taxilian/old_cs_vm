
#include <iostream>
#include "LexicalParser.h"

int main(int argc, char *argv[] )
{
    if (argc < 2) {
        std::cout << "usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }
    LexicalParser parser(argv[1]);
    
    Token cur;
    bool lastWasWhitespace = false;
    do {
        cur = parser.nextToken();
        if (cur.type == Token::TT_WHITESPACE) {
            // Ignore whitespace for now
            //if (!lastWasWhitespace) {
            //    std::cout << cur.toString() << std::endl;
            //    lastWasWhitespace = true;
            //}
        } else {
            lastWasWhitespace = false;
            std::cout << cur.toString() << std::endl;
        }
    } while (cur.type != Token::TT_ENDOFFILE);

    return 0;
}