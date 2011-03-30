
#include <iostream>
#include "LexicalParser.h"
#include "CodeParser.h"
#include "ICodeWriter.h"
#include "TCodeWriter.h"

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
    ICodeWriter writer("TEMP_FILE.icd");
    syntax.setLexer(&lexer2);
    syntax.setPass(2);
    syntax.setCodeWriter(&writer);
    lexer2.nextToken();
    do {
        syntax.compilation_unit();   
    } while (lexer2.current().type != TT_ENDOFFILE);

    TCodeWriter twriter("TEMP_FILE.icd", std::string(argv[1]) + ".out");
    twriter.setSymbolNameMap(syntax.getSymbolNameMap());
    twriter.setSymbolIdMap(syntax.getSymbolIdMap());
    twriter.start();

    return 0;
}
