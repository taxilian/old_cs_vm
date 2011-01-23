
#include <iostream>
#include "LexicalParser.h"
#include "SyntaxParser.h"

int main(int argc, char *argv[] )
{
    if (argc < 2) {
        std::cout << "usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }
    LexicalParser lexer(argv[1]);
    SyntaxParser syntax(lexer);

    lexer.nextToken();
    do {
        syntax.compilation_unit();   
    } while (lexer.current().type != TT_ENDOFFILE);

    return 0;
}