#include "SymbolEntry.h"
#include <boost/algorithm/string.hpp>

#include "CodeParser.h"
#include <boost/smart_ptr/make_shared.hpp>

CodeParser::CodeParser(LexicalParser* lexer) : lexer(lexer), nextId(1000), pass(1)
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


CodeParser::~CodeParser(void)
{
}

bool CodeParser::is_in_set( const std::string& needle, const std::set<std::string>& haystack )
{
    return haystack.find(needle) != haystack.end();
}

bool CodeParser::is_reserved( const std::string& kw )
{
    return is_in_set(kw, validKeywords)
        || is_in_set(kw, validTypes)
        || is_in_set(kw, validModifiers);
}

void CodeParser::raiseError(const std::string& type, const Token& found)
{
    std::stringstream ss;
    ss << "Expected: " << type << ", found: " << found.toString();
    throw SyntaxParserException(ss.str());
}

void CodeParser::assert_type( const TokenType type )
{
    if (lexer->current().type != type) {
        std::stringstream ss;
        ss << "Expected type " << type << " but found " << lexer->current().type;
        throw SyntaxParserException(ss.str());
    }
}

void CodeParser::assert_type_value( const TokenType type, const std::string& value )
{
    if (lexer->current().type != type || lexer->current().text != value) {
        std::stringstream ss;
        ss << "Expected " << value << " but found " << lexer->current().text;
        throw SyntaxParserException(ss.str());
    }
}

// Start point
void CodeParser::compilation_unit()
{
    while (true) {
        if (lexer->current().type == TT_KEYWORD
            && lexer->current().text == "class") {
            class_declaration();
        } else if (lexer->current().type == TT_KEYWORD
            && lexer->current().text == "void") {
            assert_type_value(TT_KEYWORD, "void"); lexer->nextToken();
            assert_type_value(TT_KEYWORD, "main"); lexer->nextToken();
            assert_type_value(TT_GROUPOPEN, "("); lexer->nextToken();
            assert_type_value(TT_GROUPCLOSE, ")"); lexer->nextToken();
            if (pass1()) {
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

void CodeParser::modifier()
{
    if (lexer->current().type == TT_KEYWORD
        && is_in_set(lexer->current().text, validModifiers)) {
        lastSeenModifier = lexer->current().text;
        lexer->nextToken();
    } else {
        raiseError("modifier", lexer->current());
    }
}

void CodeParser::class_name()
{
    if (lexer->current().type == TT_KEYWORD
        && !is_reserved(lexer->current().text)) {
        lastSeenName = lexer->current().text;
        lexer->nextToken();
    } else {
        raiseError("class_name", lexer->current());
    }
}

void CodeParser::type()
{
    if (lexer->current().type == TT_KEYWORD
        && (is_in_set(lexer->current().text, validTypes)
            || !is_reserved(lexer->current().text))) {
        lastSeenType = lexer->current().text;
        if (pass2())
            typePush(lastSeenType);
        lexer->nextToken();
        return;
    }
    raiseError("typename", lexer->current());
}

void CodeParser::character_literal()
{
    if (lexer->current().type == TT_CHAR) {
        if (pass2())
            litPush(lexer->current().text, "char");
        lexer->nextToken();
    } else {
        raiseError("character_literal", lexer->current());
    }
}

void CodeParser::numeric_literal()
{
    bool pos = true;
    if (lexer->current().type == TT_OPERATOR) {
        if (lexer->current().text == "+"
            || lexer->current().text == "-") {
            if (lexer->current().text == "-")
                pos = false;
            lexer->nextToken();
        } else {
            raiseError("numeric_literal", lexer->current());
        }
    }
    number(pos);
}

void CodeParser::number(bool pos/* = true*/)
{
    if (lexer->current().type == TT_NUMBER) {
        std::string num = (pos ? "+" : "-") + lexer->current().text;
        if (pass2())
            litPush(num, "int");
        lexer->nextToken();
    } else {
        raiseError("number", lexer->current());
    }
}

void CodeParser::method_body()
{
    assert_type_value(TT_GROUPOPEN, "{");
    lexer->nextToken();

    variable_declaration();

    while (lexer->current().text != "}") {
        statement();
    }

    assert_type_value(TT_GROUPCLOSE, "}");
    lexer->nextToken();
}

void CodeParser::variable_declaration()
{
    while (lexer->current().type == TT_KEYWORD
        && (is_in_set(lexer->current().text, validTypes)
            || !is_reserved(lexer->current().text))
        && lexer->peekToken(0).type == TT_KEYWORD
        && !is_reserved(lexer->peekToken(0).text)) {
        type();
        if (pass2())
            typeExist(lastSeenType);
        identifier();
        if (lexer->current().type == TT_GROUPOPEN) {
            assert_type_value(TT_GROUPOPEN, "[");
            lexer->nextToken();
            assert_type_value(TT_GROUPCLOSE, "]");
            lexer->nextToken();
        }
        if (pass2())
            varPush(lastSeenName, lastSeenType);
        if (lexer->current().type == TT_OPERATOR
            && lexer->current().text == "=") {
            lexer->nextToken();
            if (pass2())
                opPush("=");

            assignment_expression();
        }
        if (pass1()) {
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
        } else if (pass2())
            end_of_expr();
        assert_type(TT_SEMICOLON);
        lexer->nextToken();
    }
}

void CodeParser::identifier()
{
    if (lexer->current().type == TT_KEYWORD && !is_reserved(lexer->current().text)) {
        lastSeenName = lexer->current().text;
        lexer->nextToken();
    } else {
        raiseError("identifier", lexer->current());
    }
}

void CodeParser::assert_is( bool param1 )
{
    if (!param1) {
        throw SyntaxParserException("Syntax Error!");
    }
}

void CodeParser::class_declaration()
{
    assert_type_value(TT_KEYWORD, "class");
    scope_type.push_back("class");
    lexer->nextToken();
    class_name();
    if (pass1()) {
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
    lexer->nextToken();
    class_member_declaration();
    assert_type_value(TT_GROUPCLOSE, "}");
    scope_type.pop_back();
    current_scope.pop_back();
    lexer->nextToken();
}

void CodeParser::class_member_declaration()
{
    while (lexer->current().text != "}") {
        assert_type(TT_KEYWORD);
        if (is_in_set(lexer->current().text, validModifiers)) {
            modifier();
            assert_type(TT_KEYWORD);
        } else {
            // Default modifier is private
            lastSeenModifier = "private";
        }
        if (lexer->peekToken(0).type == TT_KEYWORD) {
            type();
            identifier();
            field_declaration();
        } else {
            constructor_declaration();
        }
    }
}

void CodeParser::constructor_declaration()
{
    class_name();
    if (pass2())
        ctordecl(lastSeenName);
    assert_type_value(TT_GROUPOPEN, "(");
    lexer->nextToken();
    current_scope.push_back(lastSeenName); 
    lastSeenFunction = lastSeenName;
    parameter_list();
    current_scope.pop_back();
    if (pass1()) {
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
    lexer->nextToken();
    current_scope.push_back(lastSeenFunction);
    method_body();
    current_scope.pop_back();
}

void CodeParser::field_declaration()
{
    lastSeenFieldType = "";
    if (lexer->current().type == TT_GROUPOPEN && lexer->current().text == "[") {
        assert_type_value(TT_GROUPOPEN, "[");
        lexer->nextToken();
        assert_type_value(TT_GROUPCLOSE, "]");
        lexer->nextToken();
        lastSeenType += "[]";
        lastSeenFieldType = "array";
    }
    if (pass2())
        varPush(lastSeenName, lastSeenType);
    if (lexer->current().type == TT_OPERATOR
        && lexer->current().text == "=") {
        if (pass2())
            opPush("=");
        lexer->nextToken();
        assignment_expression();
        lastSeenFieldType += " assign";
    }
    if (lexer->current().type == TT_SEMICOLON) {
        // it's a member variable
        lexer->nextToken();
        if (pass1()) {
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
        } else if (pass2()) {
            end_of_expr();
        }
        return; // If it's a variable we stop here
    }
    lastSeenFieldType = "function";
    lastSeenFunction = lastSeenName;
    assert_type_value(TT_GROUPOPEN, "(");
    lexer->nextToken();
    // Add the function name to the scope
    current_scope.push_back(lastSeenName); 
    parameter_list();
    current_scope.pop_back();
    if (pass1()) {
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
    lexer->nextToken();
    current_scope.push_back(lastSeenFunction);
    method_body();
    current_scope.pop_back();
}

void CodeParser::parameter_list()
{
    foundParams.clear();
    while (lexer->current().text != ")") {
        parameter();
        if (pass1()) {
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

            ParameterDefPtr param = boost::make_shared<ParameterDef>(); 
            param->paramId = symb->id;
            foundParams.push_back(param);
        }
        if (lexer->current().text == ",") {
            lexer->nextToken();
        }
    }
}

void CodeParser::parameter()
{
    type();
    if (pass2())
        typeExist(lastSeenType);
    identifier();
    if (lexer->current().type == TT_GROUPOPEN) {
        assert_type_value(TT_GROUPOPEN, "[");
        lexer->nextToken();
        assert_type_value(TT_GROUPCLOSE, "]");
        lexer->nextToken();
    }
}

void CodeParser::statement()
{
    std::string& curTxt(lexer->current().text);
    if (lexer->current().type == TT_GROUPOPEN
        && curTxt == "{") {
        lexer->nextToken();
        while (lexer->current().text != "}") { 
            statement();
        }
        assert_type_value(TT_GROUPCLOSE, "}");
        lexer->nextToken();
        return;
    } else if (curTxt == "if") {
        lexer->nextToken();
        cmd_if();
    } else if (curTxt == "while") {
        lexer->nextToken();
        cmd_while();
    } else if (curTxt == "return") {
        lexer->nextToken();
        cmd_return();
    } else if (curTxt == "cout") {
        lexer->nextToken();
        cmd_cout();
    } else if (curTxt == "cin") {
        lexer->nextToken();
        cmd_cin();
    } else {
        expression();
        assert_type(TT_SEMICOLON);
        end_of_expr();
        lexer->nextToken();
    }
}

void CodeParser::cmd_if()
{
    assert_type_value(TT_GROUPOPEN, "(");
    if (pass2())
        opPush("(");
    lexer->nextToken();
    expression();
    assert_type_value(TT_GROUPCLOSE, ")");
    lexer->nextToken();
    if (pass2()) {
        closeParen();
        keyword_sa("if");
    }
    statement();
    if (lexer->current().type == TT_KEYWORD
        && lexer->current().text == "else") {
        lexer->nextToken();
        statement();
    }
}

void CodeParser::cmd_while()
{
    assert_type_value(TT_GROUPOPEN, "(");
    if (pass2())
        opPush("(");
    lexer->nextToken();
    expression();
    assert_type_value(TT_GROUPCLOSE, ")");
    lexer->nextToken();
    if (pass2()) {
        closeParen();
        keyword_sa("while");
    }
    statement();
}

void CodeParser::cmd_return()
{
    expression();
    assert_type(TT_SEMICOLON);
    if (pass2()) {
        end_of_expr();
        keyword_sa("return");
    }
    lexer->nextToken();
}

void CodeParser::cmd_cout()
{
    assert_type_value(TT_OPERATOR, "<<");
    lexer->nextToken();
    expression();
    assert_type(TT_SEMICOLON);
    lexer->nextToken();
    if (pass2()) {
        end_of_expr();
        keyword_sa("cout");
    }
}

void CodeParser::cmd_cin()
{
    assert_type_value(TT_OPERATOR, ">>");
    expression();
    assert_type(TT_SEMICOLON);
    lexer->nextToken();
    if (pass2()) {
        end_of_expr();
        keyword_sa("cin");
    }
}

void CodeParser::expression()
{
    std::string curTxt = lexer->current().text;
    if (lexer->current().type == TT_GROUPOPEN && curTxt == "(") {
        lexer->nextToken();
        if (pass2())
            opPush("(");
        expression();
        assert_type_value(TT_GROUPCLOSE, ")");
        lexer->nextToken();
        if (pass2())
            closeParen();
        if (lexer->current().type == TT_OPERATOR)
            expressionz();
    } else if (curTxt == "true") {
        lexer->nextToken();
        if (pass2())
            litPush("true", "bool");
        if (lexer->current().type == TT_OPERATOR)
            expressionz();
    } else if (curTxt == "false") {
        lexer->nextToken();
        if (pass2())
            litPush("false", "bool");
        if (lexer->current().type == TT_OPERATOR)
            expressionz();
    } else if (curTxt == "null") {
        lexer->nextToken();
        if (pass2())
            litPush("null", "null");
        if (lexer->current().type == TT_OPERATOR)
            expressionz();
    } else if (lexer->current().type == TT_NUMBER
        || (lexer->current().type == TT_OPERATOR && (curTxt == "-" || curTxt == "+")
            && lexer->peekToken().type == TT_NUMBER)) {
        numeric_literal();
        if (lexer->current().type == TT_OPERATOR)
            expressionz();
    } else if (lexer->current().type == TT_CHAR) {
        character_literal();
        if (lexer->current().type == TT_OPERATOR)
            expressionz();
    } else {
        identifier();
        if (pass2())
            idPush(lastSeenName);
        if (lexer->current().type == TT_GROUPOPEN) {
            fn_arr_member();
        }
        if (pass2())
            idExist();
        if (lexer->current().type == TT_OPERATOR && lexer->current().text == ".") {
            member_refz();
        }
        if (lexer->current().type == TT_OPERATOR)
            expressionz();
    }
}

void CodeParser::expressionz()
{
    assert_type(TT_OPERATOR);
    const std::string& curTxt = lexer->current().text;
    if (curTxt == "=") {
        lexer->nextToken();
        if (pass2())
            opPush(curTxt);
        assignment_expression();
    } else if (curTxt == "&&") {
        lexer->nextToken();
        if (pass2())
            opPush(curTxt);
        expression();
    } else if (curTxt == "||") {
        lexer->nextToken();
        if (pass2())
            opPush(curTxt);
        expression();
    } else if (curTxt == "==") {
        lexer->nextToken();
        if (pass2())
            opPush(curTxt);
        expression();
    } else if (curTxt == "!=") {
        lexer->nextToken();
        if (pass2())
            opPush(curTxt);
        expression();
    } else if (curTxt == "<=") {
        lexer->nextToken();
        if (pass2())
            opPush(curTxt);
        expression();
    } else if (curTxt == ">=") {
        lexer->nextToken();
        if (pass2())
            opPush(curTxt);
        expression();
    } else if (curTxt == "<") {
        lexer->nextToken();
        if (pass2())
            opPush(curTxt);
        expression();
    } else if (curTxt == ">") {
        lexer->nextToken();
        if (pass2())
            opPush(curTxt);
        expression();
    } else if (curTxt == "+") {
        lexer->nextToken();
        if (pass2())
            opPush(curTxt);
        expression();
    } else if (curTxt == "-") {
        lexer->nextToken();
        if (pass2())
            opPush(curTxt);
        expression();
    } else if (curTxt == "*") {
        lexer->nextToken();
        if (pass2())
            opPush(curTxt);
        expression();
    } else if (curTxt == "/") {
        lexer->nextToken();
        if (pass2())
            opPush(curTxt);
        expression();
    } else {
        raiseError("right-hand expression", lexer->current());
    }
}

void CodeParser::assignment_expression()
{
    const std::string& curTxt = lexer->current().text;
    if (curTxt == "this") {
        lexer->nextToken();
    } else if (curTxt == "new") {
        lexer->nextToken();
        type();
        new_declaration();
    } else if (curTxt == "atoi") {
        lexer->nextToken();
        assert_type_value(TT_GROUPOPEN, "(");
        if (pass2())
            opPush("(");
        lexer->nextToken();
        expression();
        if (pass2()) {
            closeParen();
            builtin_sa("atoi");
        }
        assert_type_value(TT_GROUPCLOSE, ")");
        lexer->nextToken();
    } else if (curTxt == "itoa") {
        lexer->nextToken();
        assert_type_value(TT_GROUPOPEN, "(");
        if (pass2())
            opPush("(");
        lexer->nextToken();
        expression();
        if (pass2()) {
            closeParen();
            builtin_sa("itoa");
        }
        assert_type_value(TT_GROUPCLOSE, ")");
        lexer->nextToken();
    } else {
        expression();
    }
}

void CodeParser::new_declaration()
{
    if (lexer->current().type == TT_GROUPOPEN
        && lexer->current().text == "(") {
        if (pass2()) {
            opPush("(");
            begArgList();
        }
        lexer->nextToken();
        argument_list();
        assert_type_value(TT_GROUPCLOSE, ")");
        if (pass2()) {
            closeParen();
            endArgList();
            newObj();
        }
    } else if (lexer->current().type == TT_GROUPOPEN
        && lexer->current().text == "[") {
        if (pass2()) {
            opPush("[");
        }
        lexer->nextToken();
        expression();
        assert_type_value(TT_GROUPCLOSE, "]");
        if (pass2()) {
            closeBracket();
            newArr();
        }
    }
}

void CodeParser::member_refz()
{
    assert_type_value(TT_OPERATOR, ".");
    lexer->nextToken();
    identifier();
    if (pass2())
        idPush(lastSeenName);
    if (lexer->current().type == TT_GROUPOPEN) {
        fn_arr_member();
    }
    if (pass2())
        refExist();
    if (lexer->current().type == TT_OPERATOR && lexer->current().text == ".") {
        member_refz();
    }
}

void CodeParser::fn_arr_member()
{
    if (lexer->current().type == TT_GROUPOPEN
        && lexer->current().text == "(") {
        argument_list();
    } else if (lexer->current().type == TT_GROUPOPEN
        && lexer->current().text == "[") {
        if (pass2())
            opPush("[");
        lexer->nextToken();
        expression();
        assert_type_value(TT_GROUPCLOSE, "]");
        lexer->nextToken();
        if (pass2()) {
            closeBracket();
            arr_sa();
        }
    }
}

void CodeParser::argument_list()
{
    assert_type_value(TT_GROUPOPEN, "(");
    lexer->nextToken();
    if (pass2()) {
        opPush("(");
        begArgList();
    }
    while (lexer->current().text != ")") {
        expression();
        if (lexer->current().text == ",") {
            if (pass2())
                this->oper_comma();
            lexer->nextToken();
        }
    }
    if (pass2()) {
        closeParen();
        endArgList();
        func_sa();
    }
    lexer->nextToken();
}

const std::string CodeParser::getScopeString()
{
    return boost::algorithm::join(current_scope, ".");
}

std::string CodeParser::makeSymbolId( const std::string& prefix )
{
    std::stringstream ss;
    ss << prefix << nextId++;
    return ss.str();
}

void CodeParser::registerSymbol( const SymbolEntryPtr& symbol )
{
    symbol_id_map[symbol->id] = symbol;
    current_scope.push_back(symbol->value);
    symbol_name_map[getScopeString()] = symbol;
    current_scope.pop_back();
}

//////////////////////////////////////
// Symantic Action stuff            //
//////////////////////////////////////

void CodeParser::typePush( const std::string& type )
{
    saStack.push_back(boost::make_shared<type_SAR>(type));
}

void CodeParser::opPush( const std::string& op )
{
    opStack.push_back(std::string(op));
}

void CodeParser::litPush( const std::string& lit, const std::string& type )
{
    if (type == "int")
        saStack.push_back(boost::make_shared<int_SAR>(lit));
    else if (type == "bool")
        saStack.push_back(boost::make_shared<bool_SAR>(lit));
    else if (type == "char")
        saStack.push_back(boost::make_shared<char_SAR>(lit));
    else if (type == "null")
        saStack.push_back(boost::make_shared<null_SAR>());
    else
        throw SyntaxParserException("Bad literal type");
}

bool CodeParser::typeExist( const std::string &type )
{
    if (is_in_set(type, validTypes) || symbol_name_map.find("g." + type) != symbol_name_map.end())
        return true;
    else
        throw SyntaxParserException("Type does not exist: " + type);
}

void CodeParser::ctordecl( const std::string& ctorname )
{
    if (ctorname != current_scope.back()) {
        throw SyntaxParserException("Method looks like a constructor but is the wrong name");
    }
}

void CodeParser::idPush( const std::string& id )
{
    saStack.push_back(boost::make_shared<id_SAR>(id));
}

void CodeParser::varPush( const std::string& name, const std::string& type )
{
    // TODO: check the var here
    saStack.push_back(boost::make_shared<var_SAR>(name, type));
}
