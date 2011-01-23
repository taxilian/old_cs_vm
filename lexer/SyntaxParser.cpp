#include "SyntaxParser.h"


SyntaxParser::SyntaxParser(LexicalParser& lexer) : lexer(lexer)
{
    std::set<std::string> validKeywords;
    std::set<std::string> validTypes;
    std::set<std::string> validModifiers;
    std::set<std::string> validOperators;

    validKeywords.insert("atoi");
    validKeywords.insert("class");
    validKeywords.insert("cin");
    validKeywords.insert("cout");
    validKeywords.insert("else");
    validKeywords.insert("false");
    validKeywords.insert("if");
    validKeywords.insert("itoa");
    validKeywords.insert("main");
    validKeywords.insert("new");
    validKeywords.insert("null");
    validKeywords.insert("object");
    validKeywords.insert("return");
    validKeywords.insert("string");
    validKeywords.insert("this");
    validKeywords.insert("true");
    validKeywords.insert("while");

    validTypes.insert("int");
    validTypes.insert("char");
    validTypes.insert("bool");
    validTypes.insert("void");

    validModifiers.insert("public");
    validModifiers.insert("private");

    validOperators.insert("=");
    validOperators.insert("&&");
    validOperators.insert("||");
    validOperators.insert("==");
    validOperators.insert("!=");
    validOperators.insert("<=");
    validOperators.insert(">=");
    validOperators.insert("<");
    validOperators.insert(">");
    validOperators.insert("+");
    validOperators.insert("-");
    validOperators.insert("*");
    validOperators.insert("/");
}


SyntaxParser::~SyntaxParser(void)
{
}

bool SyntaxParser::is_in_set( const std::string& needle, const std::set<std::string>& haystack )
{
    return haystack.find(needle) != haystack.end();
}

bool SyntaxParser::is_reserved( const std::string& kw )
{
    return is_in_set(kw, validKeywords)
        || is_in_set(kw, validTypes)
        || is_in_set(kw, validModifiers);
}

void SyntaxParser::raiseError(const std::string& type, const Token& found)
{
    std::stringstream ss;
    ss << "Expected: " << type << ", found: " << found.toString();
    throw SyntaxParserException(ss.str());
}

void SyntaxParser::assert_type( const TokenType type )
{
    if (lexer.current().type != type) {
        std::stringstream ss;
        ss << "Expected type " << type << " but found " << lexer.current().type;
        throw SyntaxParserException(ss.str());
    }
}

void SyntaxParser::assert_type_value( const TokenType type, const std::string& value )
{
    if (lexer.current().type != type || lexer.current().text != value) {
        std::stringstream ss;
        ss << "Expected " << value << " but found " << lexer.current().text;
        throw SyntaxParserException(ss.str());
    }
}


void SyntaxParser::compilation_unit()
{
    if (lexer.current().type == TT_KEYWORD
        && lexer.current().text == "class") {
        class_declaration();
    } else {
        assert_type_value(TT_KEYWORD, "void"); lexer.nextToken();
        assert_type_value(TT_KEYWORD, "main"); lexer.nextToken();
        assert_type_value(TT_GROUPOPEN, "("); lexer.nextToken();
        assert_type_value(TT_GROUPCLOSE, ")"); lexer.nextToken();
        method_body();
    }
}

void SyntaxParser::modifier()
{
    if (lexer.current().type == TT_KEYWORD
        && is_in_set(lexer.current().text, validModifiers)) {
        lexer.nextToken();
    }
    raiseError("modifier", lexer.current());
}

void SyntaxParser::class_name()
{
    if (lexer.current().type == TT_KEYWORD
        && !is_reserved(lexer.current().text)) {
        lexer.nextToken();
    }
    raiseError("class_name", lexer.current());
}

void SyntaxParser::type()
{
    if (lexer.current().type == TT_KEYWORD
        && (is_in_set(lexer.current().text, validTypes)
            || !is_reserved(lexer.current().text))) {
        lexer.nextToken();
        return;
    }
    raiseError("typename", lexer.current());
}

void SyntaxParser::character_literal()
{
    if (lexer.current().type == TT_CHAR) {
        lexer.nextToken();
    }
    raiseError("character_literal", lexer.current());
}

void SyntaxParser::numeric_literal()
{
    if (lexer.current().type == TT_OPERATOR) {
        if (lexer.current().text == "+"
            || lexer.current().text == "-") {
            lexer.nextToken();
        } else {
            raiseError("numeric_literal", lexer.current());
        }
    }
    number();
}

void SyntaxParser::number()
{
    if (lexer.current().type == TT_NUMBER) {
        lexer.nextToken();
    }
    raiseError("number", lexer.current());
}

void SyntaxParser::method_body()
{
    assert_type_value(TT_GROUPOPEN, "{");
    lexer.nextToken();

    variable_declaration();

    statement();

    assert_type_value(TT_GROUPCLOSE, "}");
    lexer.nextToken();
}

void SyntaxParser::variable_declaration()
{
    type();
    identifier();
    if (lexer.current().type == TT_GROUPOPEN) {
        assert_type_value(TT_GROUPOPEN, "[");
        lexer.nextToken();
        assert_type_value(TT_GROUPCLOSE, "]");
        lexer.nextToken();
    }
    if (lexer.current().type == TT_OPERATOR
        && lexer.current().text == "=") {
        lexer.nextToken();
        assignment_expression();
    }
    assert_type(TT_SEMICOLON);
}

void SyntaxParser::identifier()
{
    if (lexer.current().type == TT_KEYWORD && !is_reserved(lexer.current().text)) {
        lexer.nextToken();
    }
    raiseError("identifier", lexer.current());
}

void SyntaxParser::assert_is( bool param1 )
{
    if (!param1) {
        throw SyntaxParserException("Syntax Error!");
    }
}

void SyntaxParser::class_declaration()
{
    assert_type_value(TT_KEYWORD, "class");
    lexer.nextToken();
    class_name();
    assert_type_value(TT_GROUPOPEN, "{");
    lexer.nextToken();
    class_member_declaration();
    assert_type_value(TT_GROUPCLOSE, "}");
    lexer.nextToken();
}

void SyntaxParser::class_member_declaration()
{
    assert_type(TT_KEYWORD);
    if (is_in_set(lexer.current().text, validModifiers)) {
        modifier();
        lexer.nextToken();
        assert_type(TT_KEYWORD);
    }
    if (lexer.peekToken(0).type == TT_KEYWORD) {
        type();
        identifier();
        field_declaration();
    } else {
        constructor_declaration();
    }
    assert_type(TT_SEMICOLON);
    lexer.nextToken();
}

void SyntaxParser::constructor_declaration()
{
    class_name();
    assert_type_value(TT_GROUPOPEN, "(");
    lexer.nextToken();
    parameter_list();
    assert_type_value(TT_GROUPCLOSE, ")");
    lexer.nextToken();
    method_body();
}

void SyntaxParser::field_declaration()
{
    if (lexer.current().type == TT_GROUPOPEN) {
        assert_type_value(TT_GROUPOPEN, "[");
        lexer.nextToken();
        assert_type_value(TT_GROUPCLOSE, "]");
        lexer.nextToken();
    }
    if (lexer.current().type == TT_OPERATOR
        && lexer.current().text == "=") {
        lexer.nextToken();
        assignment_expression();
    }
    if (lexer.current().type == TT_SEMICOLON) {
        // it's a member variable
        lexer.nextToken();
        return; // If it's a variable we stop here
    }
    assert_type_value(TT_GROUPOPEN, "(");
    lexer.nextToken();
    parameter_list();
    assert_type_value(TT_GROUPCLOSE, ")");
    lexer.nextToken();
    method_body();
}

void SyntaxParser::parameter_list()
{
    parameter();
    if (lexer.current().type == TT_OPERATOR
        && lexer.current().text == ",") {
        lexer.nextToken();
        parameter();
    }
}

void SyntaxParser::parameter()
{
    type();
    identifier();
    if (lexer.current().type == TT_GROUPOPEN) {
        assert_type_value(TT_GROUPOPEN, "[");
        lexer.nextToken();
        assert_type_value(TT_GROUPCLOSE, "]");
        lexer.nextToken();
    }
}

void SyntaxParser::statement()
{
    std::string& curTxt(lexer.current().text);
    if (lexer.current().type == TT_GROUPOPEN
        && curTxt == "}") {
        lexer.nextToken();
        statement();
        assert_type_value(TT_GROUPCLOSE, "}");
        lexer.nextToken();
        return;
    } else if (curTxt == "if") {
        lexer.nextToken();
        cmd_if();
    } else if (curTxt == "while") {
        lexer.nextToken();
        cmd_while();
    } else if (curTxt == "return") {
        lexer.nextToken();
        cmd_return();
    } else if (curTxt == "cout") {
        lexer.nextToken();
        cmd_cout();
    } else if (curTxt == "cin") {
        lexer.nextToken();
        cmd_cin();
    }
}

void SyntaxParser::cmd_if()
{
    assert_type_value(TT_GROUPOPEN, "(");
    lexer.nextToken();
    expression();
    assert_type_value(TT_GROUPCLOSE, ")");
    lexer.nextToken();
    statement();
    if (lexer.current().type == TT_KEYWORD
        && lexer.current().text == "else") {
        lexer.nextToken();
        statement();
    }
}

void SyntaxParser::cmd_while()
{
    assert_type_value(TT_GROUPOPEN, "(");
    lexer.nextToken();
    expression();
    assert_type_value(TT_GROUPCLOSE, ")");
    lexer.nextToken();
    statement();
}

void SyntaxParser::cmd_return()
{
    expression();
    assert_type(TT_SEMICOLON);
    lexer.nextToken();
}

void SyntaxParser::cmd_cout()
{
    assert_type_value(TT_OPERATOR, "<<");
    expression();
    assert_type(TT_SEMICOLON);
    lexer.nextToken();
}

void SyntaxParser::cmd_cin()
{
    assert_type_value(TT_OPERATOR, ">>");
    expression();
    assert_type(TT_SEMICOLON);
    lexer.nextToken();
}

void SyntaxParser::expression()
{
    std::string curTxt = lexer.current().text;
    if (lexer.current().type == TT_GROUPOPEN && curTxt == "(") {
        lexer.nextToken();
        expression();
        assert_type_value(TT_GROUPCLOSE, ")");
        lexer.nextToken();
        if (lexer.current().type == TT_OPERATOR)
            expressionz();
    } else if (curTxt == "true") {
        lexer.nextToken();
        if (lexer.current().type == TT_OPERATOR)
            expressionz();
    } else if (curTxt == "false") {
        lexer.nextToken();
        if (lexer.current().type == TT_OPERATOR)
            expressionz();
    } else if (curTxt == "null") {
        lexer.nextToken();
        if (lexer.current().type == TT_OPERATOR)
            expressionz();
    } else if (lexer.current().type == TT_NUMBER
        || (lexer.current().type == TT_OPERATOR && (curTxt == "-" || curTxt == "+")
            && lexer.peekToken().type == TT_NUMBER)) {
        numeric_literal();
        if (lexer.current().type == TT_OPERATOR)
            expressionz();
    } else if (lexer.current().type == TT_CHAR) {
        character_literal();
        if (lexer.current().type == TT_OPERATOR)
            expressionz();
    } else {
        identifier();
        if (lexer.current().type == TT_GROUPOPEN) {
            fn_arr_member();
        }
        if (lexer.current().type == TT_OPERATOR && lexer.current().text == ".") {
            member_refz();
        }
        if (lexer.current().type == TT_OPERATOR)
            expressionz();
    }
}

void SyntaxParser::expressionz()
{
    assert_type(TT_OPERATOR);
    const std::string& curTxt = lexer.current().text;
    if (curTxt == "=") {
        lexer.nextToken();
        assignment_expression();
    } else if (curTxt == "&&") {
        expression();
    } else if (curTxt == "||") {
        expression();
    } else if (curTxt == "==") {
        expression();
    } else if (curTxt == "!=") {
        expression();
    } else if (curTxt == "<=") {
        expression();
    } else if (curTxt == ">=") {
        expression();
    } else if (curTxt == "<") {
        expression();
    } else if (curTxt == ">") {
        expression();
    } else if (curTxt == "+") {
        expression();
    } else if (curTxt == "-") {
        expression();
    } else if (curTxt == "*") {
        expression();
    } else if (curTxt == "/") {
        expression();
    } else {
        raiseError("right-hand expression", lexer.current());
    }
}

void SyntaxParser::assignment_expression()
{
    const std::string& curTxt = lexer.current().text;
    if (curTxt == "this") {
        lexer.nextToken();
    } else if (curTxt == "new") {
        lexer.nextToken();
        type();
        new_declaration();
    } else if (curTxt == "atoi") {
        lexer.nextToken();
        assert_type_value(TT_GROUPOPEN, "(");
        lexer.nextToken();
        expression();
        assert_type_value(TT_GROUPCLOSE, ")");
        lexer.nextToken();
    } else if (curTxt == "itoa") {
        lexer.nextToken();
        assert_type_value(TT_GROUPOPEN, "(");
        lexer.nextToken();
        expression();
        assert_type_value(TT_GROUPCLOSE, ")");
        lexer.nextToken();
    } else {
        expression();
    }
}

void SyntaxParser::new_declaration()
{
    if (lexer.current().type == TT_GROUPOPEN
        && lexer.current().text == "(") {
        lexer.nextToken();
        argument_list();
        assert_type_value(TT_GROUPCLOSE, ")");
    } else if (lexer.current().type == TT_GROUPOPEN
        && lexer.current().text == "[") {
        lexer.nextToken();
        expression();
        assert_type_value(TT_GROUPCLOSE, "]");
    }
}

void SyntaxParser::member_refz()
{
    assert_type_value(TT_OPERATOR, ".");
    lexer.nextToken();
    identifier();
    if (lexer.current().type == TT_GROUPOPEN) {
        fn_arr_member();
    }
    if (lexer.current().type == TT_OPERATOR && lexer.current().text == ".") {
        member_refz();
    }
}

void SyntaxParser::fn_arr_member()
{
    if (lexer.current().type == TT_GROUPOPEN
        && lexer.current().text == "(") {
        lexer.nextToken();
        argument_list();
        assert_type_value(TT_GROUPCLOSE, ")");
    } else if (lexer.current().type == TT_GROUPOPEN
        && lexer.current().text == "[") {
        lexer.nextToken();
        expression();
        assert_type_value(TT_GROUPCLOSE, "]");
    }
}

void SyntaxParser::argument_list()
{
    expression();
    if (lexer.current().type == TT_OPERATOR
        && lexer.current().text == ",") {
        lexer.nextToken();
        argument_list();
    }
}
