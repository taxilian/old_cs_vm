
#include <iostream>
#include "LexicalParser.h"
#include "CodeParser.h"

int main(int argc, char *argv[] )
{
    if (argc < 2) {
        std::cout << "usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }
    LexicalParser lexer1(argv[1]);
    CodeParser syntax(&lexer1);

    lexer1.nextToken();
    do {
        syntax.compilation_unit();   
    } while (lexer1.current().type != TT_ENDOFFILE);

    LexicalParser lexer2(argv[1]);
    syntax.setLexer(&lexer2);
    syntax.setPass(2);
    lexer2.nextToken();
    do {
        syntax.compilation_unit();   
    } while (lexer2.current().type != TT_ENDOFFILE);

    return 0;
}