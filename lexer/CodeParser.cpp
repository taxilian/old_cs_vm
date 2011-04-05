#include <boost/algorithm/string.hpp>
#include <boost/smart_ptr/make_shared.hpp>

#include "CodeParser.h"
#include "SymbolEntry.h"
#include "ICodeWriter.h"

#include <vector>
#include <boost/lexical_cast.hpp>
#include <iostream>

using namespace std;

class scoped_PushScope
{
public:
    scoped_PushScope(CodeParser* parser, const std::string scope) : parser(parser), scope(scope), valid(true) {
        parser->current_scope.push_back(scope);
    }
    ~scoped_PushScope() {
        if (valid)
            reset();
    }
    void reset() {
        parser->current_scope.pop_back();
        assert(valid);
        valid = false;
    }
    void reactivate() {
        assert(!valid);
        parser->current_scope.push_back(scope);
        valid = true;
    }

private:
    CodeParser* parser;
    std::string scope;
    bool valid;
};

CodeParser::CodeParser(LexicalParser* lexer) : lexer(lexer), nextId(1000), pass(1), icode(NULL), lastLine(-1), failed(false), mainWritten(false)
{
    current_scope.push_back("g");

    validKeywords.insert("atoi");
    validKeywords.insert("const std::string& param1, const std::string& param2, const std::string& param3class");
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

    operatorPrecedence["*"] = 1;
    operatorPrecedence["/"] = 1;
    operatorPrecedence["+"] = 2;
    operatorPrecedence["-"] = 2;
    operatorPrecedence["<"] = 3;
    operatorPrecedence[">"] = 3;
    operatorPrecedence["<="] = 3;
    operatorPrecedence[">="] = 3;
    operatorPrecedence["=="] = 4;
    operatorPrecedence["!="] = 4;
    operatorPrecedence["&&"] = 5;
    operatorPrecedence["||"] = 5;
    operatorPrecedence["="] = 6;
    operatorPrecedence[","] = 7;
    operatorPrecedence["("] = 7;
    operatorPrecedence["["] = 7;
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

void CodeParser::LineComment()
{
    if (pass2() && lastLine != lexer->getLineNumber()) {
        lastLine = lexer->getLineNumber();
        std::stringstream ss;
        ss << "Line " << lastLine << ": ";
        ss << lexer->getLine(lastLine);
        icode->Comment(ss.str(), false);
    }
}

// Start point
void CodeParser::compilation_unit()
{
    if (pass2() && !mainWritten) {
        scoped_PushScope _s(this, "main");
        std::string scope(getScopeString());
        icode->Comment("Calling function main");
        icode->Label("START");
        icode->Write("FRAME", symbol_name_map[scope]->id, "0");
        icode->Write("CALL", "FN_" + symbol_name_map[scope]->id);
        icode->Write("TRP", "0");
    }
    while (true) {
        if (lexer->current().type == TT_KEYWORD
            && lexer->current().text == "class") {
            LineComment();
            class_declaration();
        } else if (lexer->current().type == TT_KEYWORD
            && lexer->current().text == "void") {
            LineComment();
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
            lastSeenName = "main";
            lastSeenFunction = "main";
            scoped_PushScope _s(this, "main");
            method_body();
        } else if (lexer->current().type != TT_ENDOFFILE) {
            throw SyntaxParserException("Expected void main or class; found " + lexer->current().toString());
        } else return;
    }
}

void CodeParser::method_body()
{
    foundReturn = false;
    assert_type_value(TT_GROUPOPEN, "{");
    lexer->nextToken();
	
    if (pass2()) {
        icode->Blank();
        icode->Blank();
        LineComment();
        std::string scope(getScopeString());
        icode->Comment("Begin function " + scope);
        icode->Label("FN_" + symbol_name_map[scope]->id);
        icode->Write("FUNC", symbol_name_map[scope]->id);
        if (scope_type.size() && scope_type.back() == "constructor") {
            std::string pscope = symbol_name_map[scope]->scope;
            for(ScopeMap::iterator it(symbol_id_map.begin()); it != symbol_id_map.end(); ++it) {
                if (it->second->scope == pscope && (it->second->id[0] == 'T' || boost::algorithm::starts_with(it->second->id, "REF"))) {
                    // Move all temporary variables from the class into the constructor,
                    // since those will have been created to assist with initialization
                    symbol_name_map.erase(symbol_name_map.find(it->second->scope + "." + it->second->id));
                    it->second->scope = scope;
                    registerSymbol(it->second);
                    as<MethodData>(symbol_name_map[scope]->data)->vars.push_back(it->second->id);
                }
            }
            icode->Comment("initialize default class values");
            icode->writeSection("constructor");
            icode->Comment("end initialization");
        }
    }
	
    variable_declaration();
	
    while (lexer->current().text != "}") {
        statement();
        if (!saStack.empty()) {
            while (!saStack.empty() && is_a<void_SAR>(saStack.back()))
                saPop();
            if (saStack.empty()) continue;
            std::cerr << "Warning: Stack not empty!" << std::endl;
            while (!saStack.empty()) {
                saPop();
            }
        }
    }

    if (pass2()) {
        icode->Comment("Return even if the code didn't tell us to earlier");
        icode->Write("RTN");
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
        LineComment();
        type();
        if (pass2())
            typeExist(lastSeenType);
        identifier();
        if (lexer->current().type == TT_GROUPOPEN) {
            assert_type_value(TT_GROUPOPEN, "[");
            lexer->nextToken();
            assert_type_value(TT_GROUPCLOSE, "]");
            lexer->nextToken();
            if (pass2()) {
                SARPtr id(saPop());
                SARPtr type(saPop());
                type->value += "[]";
                saStack.push_back(type);
                saStack.push_back(id);
            }
        }
        if (pass2())
            varPush(lastSeenName);
        if (lexer->current().type == TT_OPERATOR
            && lexer->current().text == "=") {
            lexer->nextToken();
            if (pass2())
                opPush("=");
			
            assignment_expression();
        } else if (pass2()) {
            saPop();
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
            as<MethodData>(symbol_name_map[symb->scope]->data)->vars.push_back(symb->id);
            registerSymbol(symb);
        } else if (pass2()) {
            end_of_expr();
        }
        assert_type(TT_SEMICOLON);
        lexer->nextToken();
    }
}

void CodeParser::class_declaration()
{
    foundConstructor = false;
    assert_type_value(TT_KEYWORD, "class");
    scope_type.push_back("class");
    lexer->nextToken();
    class_name();
    std::string className = lastSeenName;
    
    if (pass1()) {
        // Create symbol table entry for class name
        SymbolEntryPtr symb = boost::make_shared<SymbolEntry>();
        symb->id = makeSymbolId("C");
        symb->kind = "class";
        symb->scope = getScopeString();
        symb->value = lastSeenName;
        symb->data = boost::make_shared<ClassData>();
        registerSymbol(symb);
    } else if (pass2()) {
		icode->Blank();
		icode->Blank();
		icode->Comment("Begin class " + lastSeenName);
	}
    scoped_PushScope _s(this, lastSeenName);
    assert_type_value(TT_GROUPOPEN, "{");
    lexer->nextToken();

    class_member_declaration();
    
    if (!foundConstructor) {
        if (pass1()) {
            // Create an empty constructor
            SymbolEntryPtr symb = boost::make_shared<SymbolEntry>();
            symb->id = makeSymbolId("S");
            symb->value = className;
            symb->kind = "constructor";
            symb->scope = getScopeString();
            MethodDataPtr methoddata = boost::make_shared<MethodData>();
            methoddata->accessMod = "public";
            methoddata->returnType = className;
            symb->data = methoddata;
            registerSymbol(symb);

            scoped_PushScope _s(this, className);
            std::string scope(getScopeString());
            std::string pscope = symbol_name_map[scope]->scope;
            for(ScopeMap::iterator it(symbol_id_map.begin()); it != symbol_id_map.end(); ++it) {
                if (it->second->scope == pscope && (it->second->id[0] == 'T' || boost::algorithm::starts_with(it->second->id, "REF"))) {
                    // Move all temporary variables from the class into the constructor,
                    // since those will have been created to assist with initialization
                    symbol_name_map.erase(symbol_name_map.find(it->second->scope + "." + it->second->id));
                    it->second->scope = scope;
                    registerSymbol(it->second);
                    as<MethodData>(symbol_name_map[scope]->data)->vars.push_back(it->second->id);
                }
            }
        } else {
            scoped_PushScope _s(this, className);
            icode->Blank();
            icode->Blank();
            icode->Comment("Begin generated constructor " + getScopeString());
            icode->Label("FN_" + symbol_name_map[getScopeString()]->id);
            icode->Write("FUNC", symbol_name_map[getScopeString()]->id);

            icode->writeSection("constructor");
            icode->Comment("end initialization");
            icode->Write("RTN");
            std::string scope(getScopeString());
            std::string pscope = symbol_name_map[scope]->scope;
            for(ScopeMap::iterator it(symbol_id_map.begin()); it != symbol_id_map.end(); ++it) {
                if (it->second->scope == pscope && (it->second->id[0] == 'T' || boost::algorithm::starts_with(it->second->id, "REF"))) {
                    // Move all temporary variables from the class into the constructor,
                    // since those will have been created to assist with initialization
                    symbol_name_map.erase(symbol_name_map.find(it->second->scope + "." + it->second->id));
                    it->second->scope = scope;
                    registerSymbol(it->second);
                    as<MethodData>(symbol_name_map[scope]->data)->vars.push_back(it->second->id);
                }
            }
        }
    }

    assert_type_value(TT_GROUPCLOSE, "}");
    scope_type.pop_back();
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
            if (pass2())
                typeExist(lastSeenType);
            identifier();
            field_declaration();
        } else {
            constructor_declaration();
        }
    }
}

void CodeParser::constructor_declaration()
{
    foundConstructor = true;
    class_name();
    if (pass2()) {
        typeExist(lastSeenName);
        ctordecl(lastSeenName);
    }
    assert_type_value(TT_GROUPOPEN, "(");
    lexer->nextToken();
    scoped_PushScope _s(this, lastSeenName);
    lastSeenFunction = lastSeenName;
    parameter_list();
    _s.reset();
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
    if (pass2()) {
        // Process all them parameter thingies
        while (saStack.size()) {
            this->saPop();
            this->saPop();
        }
    }
    lexer->nextToken();
    _s.reactivate();
    scope_type.push_back("constructor");
    method_body();
    scope_type.pop_back();
}

void CodeParser::field_declaration()
{
    lastSeenFieldType = "";
    if (lexer->current().type == TT_GROUPOPEN && lexer->current().text == "[") {
        assert_type_value(TT_GROUPOPEN, "[");
        lexer->nextToken();
        assert_type_value(TT_GROUPCLOSE, "]");
        lexer->nextToken();
        if (pass2()) {
            SARPtr id(saPop());
            SARPtr type(saPop());
            type->value += "[]";
            saStack.push_back(type);
            saStack.push_back(id);
        }
        lastSeenType += "[]";
        lastSeenFieldType = "array";
    }
    if (pass2()) {
        varPush(lastSeenName);
        icode->beginSection("constructor");
    }
    if (lexer->current().type == TT_OPERATOR
        && lexer->current().text == "=") {
        if (pass2())
            opPush("=");
        lexer->nextToken();
        assignment_expression();
        lastSeenFieldType += " assign";
    } else if (pass2())
        saPop();

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
            as<ClassData>(symbol_name_map[symb->scope]->data)->vars.push_back(symb->id);
        } else if (pass2()) {
            end_of_expr();
        }
        if (pass2())
            icode->endSection();
        return; // If it's a variable we stop here
    }
    if (pass2())
        icode->endSection();
    lastSeenFieldType = "function";
    lastSeenFunction = lastSeenName;
    assert_type_value(TT_GROUPOPEN, "(");
    lexer->nextToken();

    // Add the function name to the scope
    scoped_PushScope _s(this, lastSeenName);
    lastSeenFuncType = lastSeenType;
    parameter_list();
    _s.reset();
    if (pass1()) {
        // Create symbol entry for the function
        SymbolEntryPtr symb = boost::make_shared<SymbolEntry>();
        symb->id = makeSymbolId("M");
        symb->value = lastSeenFunction;
        symb->kind = "method";
        symb->scope = getScopeString();
        MethodDataPtr methoddata = boost::make_shared<MethodData>();
        methoddata->accessMod = lastSeenModifier;
        methoddata->returnType = lastSeenFuncType;
        methoddata->Parameters = foundParams;
        symb->data = methoddata;
        registerSymbol(symb);
    } else if (pass2()) {
        while (saStack.size() > 1) {
            saPop(); // Pop off each parameter;
            saPop(); // we don't need them
        }
        if (saStack.size()) {
            saPop();     // Finally pop off the type of this function; we know it
        }
    }
    assert_type_value(TT_GROUPCLOSE, ")");
    lexer->nextToken();
    _s.reactivate();
    method_body();
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
			
            foundParams.push_back(symb->id);
        }
        if (lexer->current().text == ",") {
            lexer->nextToken();
        }
    }
}

void CodeParser::statement()
{
    LineComment();
    std::string& curTxt(lexer->current().text);
    try {
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
            if (pass2())
                end_of_expr();
            lexer->nextToken();
        }
    } catch (const std::exception& ex) {
        if (pass1()) {
            std::cerr << "Syntax error on line " << getLineNumber() << ":";
        } else {
            std::cerr << "Semantic error on line " << getLineNumber() << ":";
        }
        std::cerr << ex.what() << std::endl;
        bool stop = false;
        while (!stop) {
            if (lexer->current().text == ";"
                || lexer->current().text == "}"
                )
                stop = true;
            if (lexer->current().type == TT_ENDOFFILE)
                throw ex;
            lexer->nextToken();

        }
        failed = true;
    }
}

void CodeParser::cmd_if()
{
    assert_type_value(TT_GROUPOPEN, "(");
    std::string ifId;
    if (pass2()) {
        opPush("(");
        icode->Comment("Begin if");
        ifId = makeSymbolId("IF");
        loop_stack.push_back(ifId);
    }
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
        if (pass2()) {
            keyword_sa("else");
        }
        statement();
        if (pass2()) {
            icode->Label(ifId + "_SEL");
        }
    } else if (pass2()) {
        icode->Label(ifId + "_SIF");
    }
    if (pass2()) {
        icode->Write("NOOP");
        icode->Comment("endif");
        loop_stack.pop_back();
    }
}

void CodeParser::cmd_while()
{
    assert_type_value(TT_GROUPOPEN, "(");
    std::string whileId;
    if (pass2()) {
        opPush("(");
        icode->Comment("Begin while");
        whileId = makeSymbolId("WH");
        loop_stack.push_back(whileId);
        icode->Label(whileId + "_ST");
    }
    lexer->nextToken();
    expression();
    assert_type_value(TT_GROUPCLOSE, ")");
    lexer->nextToken();
    if (pass2()) {
        closeParen();
        keyword_sa("while");
    }
    statement();
    if (pass2()) {
        icode->Write("JMP", whileId + "_ST");
        icode->Label(whileId + "_END");
        icode->Write("NOOP");
        loop_stack.pop_back();
    }
}

void CodeParser::cmd_return()
{
    if (lexer->current().type != TT_SEMICOLON) {
        expression();
    } else {
        if (pass2())
            saStack.push_back(boost::make_shared<void_SAR>());
    }
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
    lexer->nextToken();
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
        if (lexer->current().type == TT_OPERATOR && lexer->current().text != ",")
            expressionz();
    } else if (curTxt == "true") {
        lexer->nextToken();
        if (pass2())
            litPush("true", "bool");
        if (lexer->current().type == TT_OPERATOR && lexer->current().text != ",")
            expressionz();
    } else if (curTxt == "false") {
        lexer->nextToken();
        if (pass2())
            litPush("false", "bool");
        if (lexer->current().type == TT_OPERATOR && lexer->current().text != ",")
            expressionz();
    } else if (curTxt == "null") {
        lexer->nextToken();
        if (pass2())
            litPush("null", "null");
        if (lexer->current().type == TT_OPERATOR && lexer->current().text != ",")
            expressionz();
    } else if (lexer->current().type == TT_NUMBER
            || (lexer->current().type == TT_OPERATOR && (curTxt == "-" || curTxt == "+")
                && lexer->peekToken().type == TT_NUMBER)) {
        numeric_literal();
        if (lexer->current().type == TT_OPERATOR && lexer->current().text != ",")
            expressionz();
    } else if (lexer->current().type == TT_CHAR) {
        character_literal();
        if (lexer->current().type == TT_OPERATOR && lexer->current().text != ",")
            expressionz();
    } else {
        identifier();
        if (pass2()) {
            idExist();
        }
        if (lexer->current().type == TT_GROUPOPEN) {
            fn_arr_member();
        }
        if (lexer->current().type == TT_OPERATOR && lexer->current().text == ".") {
            member_refz();
        }
        if (lexer->current().type == TT_OPERATOR && lexer->current().text != ",")
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
    if (curTxt == "new") {
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
        argument_list();
        if (pass2()) {
            newObj();
        }
    } else if (lexer->current().type == TT_GROUPOPEN
			   && lexer->current().text == "[") {
        if (pass2()) {
            opPush("[");
            saStack.back()->value += "[]";
        }
        lexer->nextToken();
        expression();
        assert_type_value(TT_GROUPCLOSE, "]");
        lexer->nextToken();
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
    if (pass2()) {
        opPush(".");
    }
    identifier();
    if (lexer->current().type == TT_GROUPOPEN) {
        fn_arr_member();
    }
    if (lexer->current().type == TT_OPERATOR && lexer->current().text == ".") {
        member_refz();
    }
}

void CodeParser::fn_arr_member()
{
    if (lexer->current().type == TT_GROUPOPEN
        && lexer->current().text == "(") {
        argument_list();
        if (pass2()) {
            func_sa();
        }
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
        begArgList();
        opPush("(");
    }
    while (lexer->current().text != ")") {
        expression();
        if (lexer->current().text == ",") {
            if (pass2())
                opPush(",");
            lexer->nextToken();
        }
    }
    if (pass2()) {
        closeParen();
        endArgList();
    }
    lexer->nextToken();
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

void CodeParser::identifier()
{
    if (lexer->current().type == TT_KEYWORD && !is_reserved(lexer->current().text)) {
        lastSeenName = lexer->current().text;
        if (pass2())
            idPush(lastSeenName);
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
    scoped_PushScope _s(this, symbol->value);
    symbol_name_map[getScopeString()] = symbol;
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
    if (op == "(" || op == "[") {
    } else if (op == ",") {
        while (opStack.back() != "," && opStack.back() != "(") {
            processOperatorStack();
        }
    } else {
        while (!opStack.empty() && operatorPrecedence[op] > operatorPrecedence[opStack.back()] ) {
            processOperatorStack();
        }
    }
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

void CodeParser::varPush( const std::string& name )
{
    SARPtr sar1(saPop());
    SARPtr sar2(saPop());
    assert(is_a<id_SAR>(sar1) && is_a<type_SAR>(sar2));
    saStack.push_back(boost::make_shared<var_SAR>(sar1->value, sar2->value));
}

std::string CodeParser::getScopeType( const std::string& scopeStr )
{
    std::map<std::string, SymbolEntryPtr>::iterator fnd = symbol_name_map.find(scopeStr);
    if(fnd->second->kind == "variable" || fnd->second->kind == "param") {
        boost::shared_ptr<TypeData> td(boost::dynamic_pointer_cast<TypeData>(fnd->second->data));
        return td->type;
    } else if (fnd->second->kind == "method") {
        MethodDataPtr methoddata = as<MethodData>(fnd->second->data);
        return methoddata->returnType;
    }
    assert(false);
    return "";
}
std::string CodeParser::getScopeType( const SARPtr& sar )
{
    if (is_a<id_SAR>(sar)) {
        if (sar->value == "this") {
            return findClass();
        }
        std::string scopeStr = findInScope(sar->value);
        if (scopeStr.empty())
            throw SyntaxParserException("Could not find '" + sar->value + "'");
        return getScopeType(scopeStr);
    } else if (is_a<typedSAR>(sar)) {
        return as<typedSAR>(sar)->getType();
    } else {
        return "";
    }
}

bool CodeParser::idExist()
{
    SARPtr sar(saPop());
    assert(is_a<id_SAR>(sar));

    if (opStack.size() && opStack.back() == ".") {
        processOperatorStack();
        assert(is_a<ref_SAR>(saStack.back()));
        boost::shared_ptr<ref_SAR> rsar(as<ref_SAR>(saStack.back()));

    } else {
        if (sar->value == "this") {
            std::string type(findClass());
            boost::shared_ptr<var_SAR> vsar(boost::make_shared<var_SAR>("this", type));
            saStack.push_back(vsar);
        } else {
            std::string scopestr = findInScope(sar->value);
            if (scopestr.empty())
                throw SyntaxParserException("Undefined symbol: " + sar->value);
            saStack.push_back(sar);
        }
    }
    return true;
}

void CodeParser::closeParen()
{
    while (opStack.back() != "(") {
        processOperatorStack();
    }
    opPop();
}

void CodeParser::closeBracket()
{
    while (opStack.back() != "[") {
        processOperatorStack();
    }
    opPop();
}

void CodeParser::end_of_expr()
{
    if (!saStack.size())
        return;

    SARPtr sar(saStack.back());
    
    while (!opStack.empty()) {
        processOperatorStack();
    }
}

bool CodeParser::compatibleTypes( const SARPtr& sar1, const SARPtr& sar2 )
{
    if (!sar1 || !sar2)
        return false;
    std::string type1(getScopeType(sar1));
    std::string type2(getScopeType(sar2));
    if (type1 == type2) {
        return true;
    } else if (type1 == "char") {
        return type2 == "char" || type2 == "int";
    } else if (!is_in_set(type1, validTypes)) {
        // Objects can be assigned to NULL
        return type2 == "null";
    } else {
        return false;
    }
}

void CodeParser::begArgList()
{
    saStack.push_back(boost::make_shared<begArgList_SAR>(""));
}

void CodeParser::endArgList()
{
    SARPtr sar(saPop());
    boost::shared_ptr<argList_SAR> al(boost::make_shared<argList_SAR>());
    while (!is_a<begArgList_SAR>(sar)) {
        al->pushArg(sar);
        sar = saPop();
    }
    saStack.push_back(al);
}

void CodeParser::func_sa()
{
    boost::shared_ptr<argList_SAR> argList(as<argList_SAR>(saPop()));
    SARPtr funcnamesar(saPop());
    SARPtr varsar;
    if (saStack.empty()) {
        varsar = boost::make_shared<id_SAR>("this");
    } else {
        varsar = saPop();
    }
    std::string type = getScopeType(varsar);
    std::string scope = "g." + type;
    if (!findFunction(scope, funcnamesar->value, argList)) {
        throw SyntaxParserException("Cannot find a method matching " + type + "." + funcnamesar->value + " with these arguments");
    }
    
    SymbolEntryPtr func(symbol_name_map[scope + "." + funcnamesar->value]);
    MethodDataPtr md(as<MethodData>(func->data));
    std::string funcId(func->id);

    std::string tempId;
    if (md->returnType != "void") {
        tempId = tempPush(getScopeType(scope + "." + funcnamesar->value));
    } else {
        saStack.push_back(boost::make_shared<void_SAR>());
    }
    
    icode->Write("FRAME", funcId, getRval(varsar));
    for (std::vector<SARPtr>::reverse_iterator it = argList->argList.rbegin();
         it != argList->argList.rend(); ++it) {
        icode->Write("PUSH", getRval(*it));
    }
    icode->Write("CALL", "FN_" + funcId);
    if (md->returnType != "void") {
        icode->Write("PEEK", tempId);
    }

    if (opStack.size())
        opPop();
}

void CodeParser::arr_sa()
{
    SARPtr idx(saPop());
    SARPtr var(saPop());

    std::string type(as<TypeData>(symbol_name_map[findInScope(var->value)]->data)->type);
    if (boost::algorithm::ends_with(type, "[]")) {
        type = type.substr(0, type.size()-2);
    }
    std::string tempVar(tempPush(type, "REF"));
    icode->Write("REF", tempVar, getRval(var), getRval(idx));
}

void CodeParser::keyword_sa( const std::string &kw )
{
    if (kw == "while") {
        SARPtr sar(saPop());
        if ((is_a<var_SAR>(sar) && as<var_SAR>(sar)->type != "bool")
            || (is_a<lit_SAR>(sar) && as<lit_SAR>(sar)->type != "bool")) {
            throw SyntaxParserException("Expecting bool and didn't find it!");
        } else {
            std::string whileId = loop_stack.back();
            icode->Write("BF", getRval(sar), whileId + "_END");
        }
    } else if (kw == "cout") {
        SARPtr sar(saPop());
        icode->Write("WRITE", getRval(sar));
    } else if (kw == "cin") {
        SARPtr sar(saPop());
        icode->Write("READ", getRval(sar));
    } else if (kw == "return") {
        SARPtr sar(saPop());
        if (sar->value == "void")
            icode->Write("RTN");
        else {
            MethodDataPtr m(as<MethodData>(symbol_name_map[getScopeString()]->data));
            if (m->returnType != getScopeType(sar))
                throw SyntaxParserException("Attempting to return " + getScopeType(sar) + " from function returning " + m->returnType);
            icode->Write("RETURN", getRval(sar));
        }
        foundReturn = true;
        // Return whatever is on the stack
    } else if (kw == "if") {
        SARPtr sar(saPop());
        if ((is_a<var_SAR>(sar) && as<var_SAR>(sar)->type != "bool")
            || (is_a<lit_SAR>(sar) && as<lit_SAR>(sar)->type != "bool")) {
            throw SyntaxParserException("Expecting bool and didn't find it!");
        } else {
            std::string ifId = loop_stack.back();
            icode->Write("BF", getRval(sar), ifId + "_SIF");
        }
    } else if (kw == "else") {
        std::string ifId = loop_stack.back();
        icode->Write("JMP", ifId + "_SEL");
        icode->Label(ifId + "_SIF");
    } else {
        throw std::runtime_error("Not implemented");
    }
}

void CodeParser::newObj()
{
    boost::shared_ptr<argList_SAR> argList(as<argList_SAR>(saPop()));
    boost::shared_ptr<type_SAR> type(as<type_SAR>(saPop()));
    std::string scope = "g." + type->value;
    if (!findFunction(scope, type->value, argList)) {
        throw SyntaxParserException("Cannot find a valid constructor for " + type->value + " with matching these arguments");
    }
    std::string var = tempPush(type->getType());
    // Allocate memory and then call the constructor
    icode->Write("NEW", boost::lexical_cast<std::string>(getTypeSize(type->getType())), var);
    if (symbol_name_map.find(scope + "." + type->value) != symbol_name_map.end()) {
        SymbolEntryPtr ctor(symbol_name_map[scope + "." + type->value]);
        icode->Write("FRAME", ctor->id, var);
        for (std::vector<SARPtr>::reverse_iterator it = argList->argList.rbegin();
             it != argList->argList.rend(); ++it) {
            icode->Write("PUSH", getRval(*it));
        }
        icode->Write("CALL", "FN_" + ctor->id);
    }
}

void CodeParser::newArr()
{
    SARPtr lenSar(saPop());
    SARPtr type(saPop());
    if (getScopeType(lenSar) != "int") {
        throw SyntaxParserException("Invalid array length; expected int, got " + getScopeType(lenSar));
    }
    std::string var = tempPush(type->value);
    icode->Write("NEW", getRval(lenSar), var);
}

void CodeParser::builtin_sa( const std::string& func )
{
    throw std::runtime_error("Not implemented");
}

std::string CodeParser::tempPush( const std::string& type, const std::string& pref/* = "T"*/ )
{
    SymbolEntryPtr symb = boost::make_shared<SymbolEntry>();
    symb->id = makeSymbolId(pref);
    symb->kind = "variable";
    symb->scope = getScopeString();
    symb->value = symb->id;
    TypeDataPtr tdata = boost::make_shared<TypeData>();
    tdata->accessMod = "private";
    tdata->type = type;
    symb->data = tdata;
    registerSymbol(symb);
    saStack.push_back(boost::make_shared<var_SAR>(symb->value, type));

    MethodDataPtr owner(as<MethodData>(symbol_name_map[symb->scope]->data));
    if (owner) {
        owner->vars.push_back(symb->id);
    }
    return symb->id;
}

bool CodeParser::findFunction( const std::string& scope, const std::string& name, const boost::shared_ptr<argList_SAR>& argList )
{
    SymbolEntryPtr parent(symbol_name_map[getScopeString()]);
    while (parent && parent->kind != "class") {
        parent = symbol_name_map[parent->scope];
    }

    std::string symName = scope + "." + name;
    if (symbol_name_map.find(symName) != symbol_name_map.end()) {
        SymbolEntryPtr s(symbol_name_map[symName]);
        if (s->data->accessMod == "private" && (!parent || s->scope != parent->scope + "." + parent->value)) 
            throw SyntaxParserException("Cannot access private method " + name + " of " + scope);
        MethodDataPtr m(as<MethodData>(s->data));
        if (m->Parameters.size() != argList->argList.size())
            return false;
        int i = 0;
        for (vector<std::string>::reverse_iterator it = m->Parameters.rbegin(); it != m->Parameters.rend(); ++it) {
            SymbolEntryPtr param(symbol_id_map[*it]);
            TypeDataPtr t(as<TypeData>(param->data));
            std::string type;
            if (is_a<lit_SAR>(argList->argList[i])) {
                type = getScopeType(argList->argList[i]);
            } else 
                type = getScopeType(findInScope(argList->argList[i]->value));
            if (t->type != type)
                return false;
            ++i;
        }
        return true;
    } else {
        return false;
    }
}

std::string getCharString(char c) {
    switch(c) {
        case '\n':
            return "\\n";
        case '\0':
            return "\\0";
        case '\r':
            return "\\r";
        default:
            return std::string() + c;
    }
}

std::string CodeParser::getRval(const SARPtr& rval) {
    if (rval->value == "this") {
        return rval->value;
    } else if (is_a<lit_SAR>(rval)) {
        if (is_a<int_SAR>(rval)) {
            return rval->value;
        } else if (is_a<bool_SAR>(rval)) {
            return rval->value == "true" ? "1" : "0";
        } else if (is_a<null_SAR>(rval)) {
            return "0";
        } else if (is_a<char_SAR>(rval)) {
            return std::string("'") + getCharString(rval->value[0]) + "'";
        } else throw SyntaxParserException("Trying to use void as an rval");
    } else if (is_a<var_SAR>(rval) || is_a<id_SAR>(rval)) {
        if (symbol_id_map.find(rval->value) != symbol_id_map.end()) {
            return rval->value;
        }
        SymbolEntryPtr right = symbol_name_map[findInScope(rval->value)]; 
        if (right->id[0] == 'V') {
            std::string rvalType = getScopeType(findInScope(rval->value));
            std::string tempId = tempPush(rvalType, "REF");
            saPop(); // We don't need it on the stack this time
            icode->Write("REF", tempId, "this", getRefval("this", rval));
            return tempId;
        } else {
            return right->id;
        }
    } else if (is_a<new_SAR>(rval)) {
        return rval->value;
    }
    return "";
}

std::string CodeParser::getRefval(const SARPtr& context, const SARPtr& rval) {
    std::string ctxId = getRval(context);
    return getRefval(ctxId, rval);
}

std::string CodeParser::getRefval(const std::string& ctxId, const SARPtr& rval)
{
    std::string ctxTypeName;
    if (ctxId == "this") {
        SymbolEntryPtr sptr(symbol_name_map[getScopeString()]);
        if (sptr->kind != "class")
            sptr = symbol_name_map[sptr->scope];
        ctxTypeName = sptr->value;
    } else {
        ctxTypeName = as<TypeData>(symbol_id_map[ctxId]->data)->type;
    }
    std::string rvSearch = "g." + ctxTypeName + "." + rval->value;
    if (symbol_name_map.find(rvSearch) != symbol_name_map.end()) {
        SymbolEntryPtr symb(symbol_name_map[rvSearch]);
        SymbolEntryPtr parent(symbol_name_map[getScopeString()]);
        while (parent->kind != "class") {
            parent = symbol_name_map[parent->scope];
        }
        if (symb->data->accessMod == "private" && symb->scope != parent->scope + "." + parent->value) {
            throw SyntaxParserException("Cannot access protected member " + rval->value + " of " + ctxTypeName);
        } else {
            return symbol_name_map[rvSearch]->id;
        }
    } else {
        throw SyntaxParserException("Cannot find a member " + rval->value + " in " + ctxTypeName);
    }
}

std::string CodeParser::getLval(const SARPtr& lval) {
    SymbolEntryPtr left = symbol_name_map[findInScope(lval->value)];
    if (left->id[0] == 'V') {
        std::string lvalType = getScopeType(findInScope(lval->value));
        std::string tempId = tempPush(lvalType, "REF");
        saPop(); // We don't need it on the stack this time
        icode->Write("REF", tempId, "this", getRefval("this", lval));
        return tempId;
    } else {
        return left->id;
    }
}

void CodeParser::processOperatorStack()
{
    std::string op = opPop();
    if (op == "==" || op == "!="
        || op == ">" || op == "<"
        || op == ">=" || op == "<="
        || op == "&&" || op == "||") {
        SARPtr sar2(saPop());
        SARPtr sar1(saPop());
        // These all require the type to be the same
        if (!compatibleTypes(sar1, sar2))
            throw SyntaxParserException("Invalid rval type: expected " + getScopeType(sar1) + " but found " + getScopeType(sar2));
        // Create a temporary variable and push it onto the stack
        std::string tempId = tempPush("bool");
        icode->DoMath(op, tempId, getRval(sar1), getRval(sar2));
    } else if (op == "(" || op == ",") {
        return;
    } else if (op == "+" || op == "-" || op == "*" || op == "/") {
        SARPtr sar2(saPop());
        SARPtr sar1(saPop());
        // These all require the type to be the same
        if (!compatibleTypes(sar1, sar2))
            throw SyntaxParserException("Invalid rval type: expected " + getScopeType(sar1) + " but found " + getScopeType(sar2));
        // Create a temporary variable and push it onto the stack
        std::string tempId = tempPush(getScopeType(sar1));
        icode->DoMath(op, tempId, getRval(sar1), getRval(sar2));
    } else if (op == "=") {
        assert(saStack.size() > 1);
        SARPtr rval(saPop());
        SARPtr lval(saPop());
        // Assignment to new variable
        if (!compatibleTypes(lval, rval)) {
            throw SyntaxParserException("Unexpected rval does not match type: " + getScopeType(lval));
        } else {
            std::string left = getLval(lval);
            std::string right = getRval(rval);
            icode->Write("MOVE", left, right);
        }
    } else if (op == ".") {
        assert(saStack.size() > 1);
        SARPtr rval(saPop());
        SARPtr lval(saPop());

        std::string type = getScopeType(lval);
        std::string scopeStr = "g." + type + "." + rval->value;
        if (symbol_name_map.find(scopeStr) == symbol_name_map.end()) {
            throw SyntaxParserException(type + " does not have a member " + rval->value);
        }
        std::string rvalType = getScopeType(scopeStr);
        std::string tempId = tempPush(rvalType, "REF");
        icode->Write("REF", tempId, getRval(lval), getRefval(lval, rval));
    }
}

std::string CodeParser::findClass()
{
    std::string str(getScopeString());
    int n=0;
    for (unsigned int i = 0; i < str.size(); ++i) {
        if (str[i] == '.') ++n;
        if (n > 1) {
            str[i] = 0; // Terminate the string here
        }
    }
    str = str.c_str();
    if (symbol_name_map[str.c_str()]->kind == "class") {
        return symbol_name_map[str]->value;
    } else {
        return "";
    }
}

std::string CodeParser::findInScope( const std::string& id )
{
    std::string scope = getScopeString();
    std::vector<std::string> scopeList;
    boost::algorithm::split(scopeList, scope, boost::algorithm::is_any_of("."));

    while (scopeList.size()) {
        std::string scopeStr = boost::algorithm::join(scopeList, ".") + "." + id;
        if (symbol_name_map.find(scopeStr) != symbol_name_map.end()) {
            return scopeStr;
        }
        scopeList.pop_back();
    }
    return std::string();
}

std::string getParentScope(const std::string& scope) {
    std::string pscope = scope.substr(0, scope.rfind('.'));
    return pscope;
}

int CodeParser::getTypeSize( const std::string& type, bool nested/* = false*/ )
{
    if (type == "bool" || type == "char") return 4;
    else if (type == "null") return 4;
    else if (type == "int") return 4;
    else if (nested) return 4; // If this type is inside another class, it's just a pointer
    else if (boost::algorithm::ends_with(type, "[]")) return 4; // arrays are pointers
    else {
        int size(0);
        std::string typeFQN("g." + type);
        SymbolEntryPtr symb(symbol_name_map[typeFQN]);
        assert(symb->kind == "class");
        ClassDataPtr cdata(as<ClassData>(symb->data));
        for (std::vector<std::string>::iterator it(cdata->vars.begin());
            it != cdata->vars.end(); ++it) {
            SymbolEntryPtr child(symbol_id_map[*it]);
            if (child->kind == "variable") {
                TypeDataPtr childType(as<TypeData>(child->data));
                size += getTypeSize(childType->type, true);
            } else { assert(false); }
        }
        return size;
    }
}

