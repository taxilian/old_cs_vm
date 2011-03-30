#include "SymbolEntry.h"
#include <boost/algorithm/string.hpp>

#include "SyntaxParser.h"
#include <boost/smart_ptr/make_shared.hpp>

SyntaxParser::SyntaxParser(LexicalParser& lexer) : lexer(lexer), nextId(1000)
{
    current_scope.push_back("g");

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

// Start point
void SyntaxParser::compilation_unit()
{
    while (true) {
        if (lexer.current().type == TT_KEYWORD
            && lexer.current().text == "class") {
            class_declaration();
        } else if (lexer.current().type == TT_KEYWORD
            && lexer.current().text == "void") {
            assert_type_value(TT_KEYWORD, "void"); lexer.nextToken();
            assert_type_value(TT_KEYWORD, "main"); lexer.nextToken();
            assert_type_value(TT_GROUPOPEN, "("); lexer.nextToken();
            assert_type_value(TT_GROUPCLOSE, ")"); lexer.nextToken();
            {
                // Create symbol entry for the main function
                SymbolEntryPtr symb = boost::make_shared<SymbolEntry>();
                symb->id = makeSymbolId("M");
                symb->value = "main";
                symb->kind = "main";
                symb->scope = getScopeString();
                MethodDataPtr methoddata = boost::make_shared<MethodData>();
                methoddata->accessMod = "public";
                methoddata->returnType = "void";
                symb->data = methoddata;
                registerSymbol(symb);
            }
            current_scope.push_back("main");
            method_body();
            current_scope.pop_back();
        } else {
            return;
        }
    }
}

void SyntaxParser::modifier()
{
    if (lexer.current().type == TT_KEYWORD
        && is_in_set(lexer.current().text, validModifiers)) {
        lastSeenModifier = lexer.current().text;
        lexer.nextToken();
    } else {
        raiseError("modifier", lexer.current());
    }
}

void SyntaxParser::class_name()
{
    if (lexer.current().type == TT_KEYWORD
        && !is_reserved(lexer.current().text)) {
        lastSeenName = lexer.current().text;
        lexer.nextToken();
    } else {
        raiseError("class_name", lexer.current());
    }
}

void SyntaxParser::type()
{
    if (lexer.current().type == TT_KEYWORD
        && (is_in_set(lexer.current().text, validTypes)
            || !is_reserved(lexer.current().text))) {
        lastSeenType = lexer.current().text;
        lexer.nextToken();
        return;
    }
    raiseError("typename", lexer.current());
}

void SyntaxParser::character_literal()
{
    if (lexer.current().type == TT_CHAR) {
        lexer.nextToken();
    } else {
        raiseError("character_literal", lexer.current());
    }
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
    } else {
        raiseError("number", lexer.current());
    }
}

void SyntaxParser::method_body()
{
    assert_type_value(TT_GROUPOPEN, "{");
    lexer.nextToken();

    variable_declaration();

    while (lexer.current().text != "}") {
        statement();
    }

    assert_type_value(TT_GROUPCLOSE, "}");
    lexer.nextToken();
}

void SyntaxParser::variable_declaration()
{
    while (lexer.current().type == TT_KEYWORD
        && (is_in_set(lexer.current().text, validTypes)
            || !is_reserved(lexer.current().text))
        && lexer.peekToken(0).type == TT_KEYWORD
        && !is_reserved(lexer.peekToken(0).text)) {
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
        {
            SymbolEntryPtr symb = boost::make_shared<SymbolEntry>();
            symb->id = makeSymbolId("L");
            symb->kind = "variable";
            symb->scope = getScopeString();
            symb->value = lastSeenName;
            TypeDataPtr tdata = boost::make_shared<TypeData>();
            tdata->accessMod = "private";
            tdata->type = lastSeenType;
            symb->data = tdata;
            registerSymbol(symb);
        }
        assert_type(TT_SEMICOLON);
        lexer.nextToken();
    }
}

void SyntaxParser::identifier()
{
    if (lexer.current().type == TT_KEYWORD && !is_reserved(lexer.current().text)) {
        lastSeenName = lexer.current().text;
        lexer.nextToken();
    } else {
        raiseError("identifier", lexer.current());
    }
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
    scope_type.push_back("class");
    lexer.nextToken();
    class_name();
    {
        // Create symbol table entry for class name
        SymbolEntryPtr symb = boost::make_shared<SymbolEntry>();
        symb->id = makeSymbolId("C");
        symb->kind = "class";
        symb->scope = getScopeString();
        symb->value = lastSeenName;
        registerSymbol(symb);
        current_scope.push_back(lastSeenName);
    }
    assert_type_value(TT_GROUPOPEN, "{");
    lexer.nextToken();
    class_member_declaration();
    assert_type_value(TT_GROUPCLOSE, "}");
    scope_type.pop_back();
    current_scope.pop_back();
    lexer.nextToken();
}

void SyntaxParser::class_member_declaration()
{
    while (lexer.current().text != "}") {
        assert_type(TT_KEYWORD);
        if (is_in_set(lexer.current().text, validModifiers)) {
            modifier();
            assert_type(TT_KEYWORD);
        } else {
            // Default modifier is private
            lastSeenModifier = "private";
        }
        if (lexer.peekToken(0).type == TT_KEYWORD) {
            type();
            identifier();
            field_declaration();
        } else {
            constructor_declaration();
        }
    }
}

void SyntaxParser::constructor_declaration()
{
    class_name();
    assert_type_value(TT_GROUPOPEN, "(");
    lexer.nextToken();
    current_scope.push_back(lastSeenName); 
    lastSeenFunction = lastSeenName;
    parameter_list();
    current_scope.pop_back();
    {
        // Create symbol entry for the function
        SymbolEntryPtr symb = boost::make_shared<SymbolEntry>();
        symb->id = makeSymbolId("S");
        symb->value = lastSeenFunction;
        symb->kind = "constructor";
        symb->scope = getScopeString();
        MethodDataPtr methoddata = boost::make_shared<MethodData>();
        methoddata->accessMod = lastSeenModifier;
        methoddata->returnType = lastSeenType;
        methoddata->Parameters = foundParams;
        symb->data = methoddata;
        registerSymbol(symb);
    }
    assert_type_value(TT_GROUPCLOSE, ")");
    lexer.nextToken();
    current_scope.push_back(lastSeenFunction);
    method_body();
    current_scope.pop_back();
}

void SyntaxParser::field_declaration()
{
    lastSeenFieldType = "";
    if (lexer.current().type == TT_GROUPOPEN && lexer.current().text == "[") {
        assert_type_value(TT_GROUPOPEN, "[");
        lexer.nextToken();
        assert_type_value(TT_GROUPCLOSE, "]");
        lexer.nextToken();
        lastSeenType += "[]";
        lastSeenFieldType = "array";
    }
    if (lexer.current().type == TT_OPERATOR
        && lexer.current().text == "=") {
        lexer.nextToken();
        assignment_expression();
        lastSeenFieldType += " assign";
    }
    if (lexer.current().type == TT_SEMICOLON) {
        // it's a member variable
        lexer.nextToken();
        {
            // Create symbol entry for the function
            SymbolEntryPtr symb = boost::make_shared<SymbolEntry>();
            symb->id = makeSymbolId("V");
            symb->value = lastSeenName;
            symb->kind = "variable";
            symb->scope = getScopeString();
            TypeDataPtr typedata = boost::make_shared<TypeData>();
            typedata->accessMod = lastSeenModifier;
            typedata->type = lastSeenType;
            symb->data = typedata;
            registerSymbol(symb);
        }
        return; // If it's a variable we stop here
    }
    lastSeenFieldType = "function";
    lastSeenFunction = lastSeenName;
    assert_type_value(TT_GROUPOPEN, "(");
    lexer.nextToken();
    // Add the function name to the scope
    current_scope.push_back(lastSeenName); 
    parameter_list();
    current_scope.pop_back();
    {
        // Create symbol entry for the function
        SymbolEntryPtr symb = boost::make_shared<SymbolEntry>();
        symb->id = makeSymbolId("M");
        symb->value = lastSeenFunction;
        symb->kind = "method";
        symb->scope = getScopeString();
        MethodDataPtr methoddata = boost::make_shared<MethodData>();
        methoddata->accessMod = lastSeenModifier;
        methoddata->returnType = lastSeenType;
        methoddata->Parameters = foundParams;
        symb->data = methoddata;
        registerSymbol(symb);
    }
    assert_type_value(TT_GROUPCLOSE, ")");
    lexer.nextToken();
    current_scope.push_back(lastSeenFunction);
    method_body();
    current_scope.pop_back();
}

void SyntaxParser::parameter_list()
{
    foundParams.clear();
    while (lexer.current().text != ")") {
        parameter();
        {
            SymbolEntryPtr symb = boost::make_shared<SymbolEntry>();
            symb->id = makeSymbolId("P");
            symb->value = lastSeenName;
            symb->kind = "param";
            symb->scope = getScopeString();
            TypeDataPtr typedata = boost::make_shared<TypeData>();
            typedata->accessMod = "private";
            typedata->type = lastSeenType;
            symb->data = typedata;
            registerSymbol(symb);

            foundParams.push_back(symb->id);
        }
        if (lexer.current().text == ",") {
            lexer.nextToken();
        }
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
        && curTxt == "{") {
        lexer.nextToken();
        while (lexer.current().text != "}") { 
            statement();
        }
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
    } else {
        expression();
        assert_type(TT_SEMICOLON);
        lexer.nextToken();
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
    lexer.nextToken();
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
        lexer.nextToken();
        expression();
    } else if (curTxt == "||") {
        lexer.nextToken();
        expression();
    } else if (curTxt == "==") {
        lexer.nextToken();
        expression();
    } else if (curTxt == "!=") {
        lexer.nextToken();
        expression();
    } else if (curTxt == "<=") {
        lexer.nextToken();
        expression();
    } else if (curTxt == ">=") {
        lexer.nextToken();
        expression();
    } else if (curTxt == "<") {
        lexer.nextToken();
        expression();
    } else if (curTxt == ">") {
        lexer.nextToken();
        expression();
    } else if (curTxt == "+") {
        lexer.nextToken();
        expression();
    } else if (curTxt == "-") {
        lexer.nextToken();
        expression();
    } else if (curTxt == "*") {
        lexer.nextToken();
        expression();
    } else if (curTxt == "/") {
        lexer.nextToken();
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
        argument_list();
    } else if (lexer.current().type == TT_GROUPOPEN
        && lexer.current().text == "[") {
        lexer.nextToken();
        expression();
        assert_type_value(TT_GROUPCLOSE, "]");
        lexer.nextToken();
    }
}

void SyntaxParser::argument_list()
{
    assert_type_value(TT_GROUPOPEN, "(");
    lexer.nextToken();
    while (lexer.current().text != ")") {
        expression();
        if (lexer.current().text == ",") {
            lexer.nextToken();
        }
    }
    lexer.nextToken();
}

const std::string SyntaxParser::getScopeString()
{
    return boost::algorithm::join(current_scope, ".");
}

std::string SyntaxParser::makeSymbolId( const std::string& prefix )
{
    std::stringstream ss;
    ss << prefix << nextId++;
    return ss.str();
}

void SyntaxParser::registerSymbol( const SymbolEntryPtr& symbol )
{
    symbol_id_map[symbol->id] = symbol;
    current_scope.push_back(symbol->value);
    symbol_name_map[getScopeString()] = symbol;
    current_scope.pop_back();
}
