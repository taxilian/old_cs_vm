
#include <iostream>
#include "LexicalParser.h"

int main(int argc, char *argv[] )
{
    LexicalParser parser("test.kxi");
    
    Token cur;
    bool lastWasWhitespace = false;
    do {
        cur = parser.nextToken();
        if (cur.type == Token::TT_WHITESPACE) {
            if (!lastWasWhitespace) {
                std::cout << cur.toString() << std::endl;
                lastWasWhitespace = true;
            }
        } else {
            lastWasWhitespace = false;
            std::cout << cur.toString() << std::endl;
        }
    } while (cur.type != Token::TT_ENDOFFILE);

    return 0;
}