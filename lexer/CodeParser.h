#pragma once
#include "LexicalParser.h"
#include <set>
#include <string>
#include <deque>
#include <map>
#include "SymbolEntry.h"
#include "SemanticActions.h"


struct SyntaxParserException : std::exception
{
    SyntaxParserException(const std::string& error)
        : m_error(error)
    { }
    ~SyntaxParserException() throw() { }
    virtual const char* what() const throw() { 
        return m_error.c_str(); 
    }
    std::string m_error;
};
typedef std::map<std::string, SymbolEntryPtr> ScopeMap;

class ICodeWriter;
class scoped_PushScope;
class CodeParser
{
public:
    CodeParser(LexicalParser* lexer);
    ~CodeParser(void);

    void setLexer(LexicalParser* lexer) { this->lexer = lexer; }
    void setCodeWriter( ICodeWriter* writer ) { icode = writer; }

    //////////////////////////////////////
    // Grammer parsing rule functions   //
    //////////////////////////////////////

    // Names, Types, Literals
    void modifier();

    void class_name();
    void type();
    void character_literal();
    void numeric_literal();
    void number(bool pos = true);
    void identifier();

    // Start symbol
    void compilation_unit();

    // Declarations
    void class_declaration();
    void class_member_declaration();
    void field_declaration();
    void constructor_declaration();
    void method_body();
    void variable_declaration();
    void parameter_list();
    void parameter();

    // Statement
    void statement();
    void cmd_if();
    void cmd_while();
    void cmd_return();
    void cmd_cout();
    void cmd_cin();

    // Expression
    void expression();
    void fn_arr_member();
    void argument_list();
    void member_refz();
    void expressionz();
    void assignment_expression();
    void new_declaration();

    ///////////////////////////////
    // Symantic Action methods   //
    ///////////////////////////////

    void idPush(const std::string& id);
    void litPush(const std::string& lit, const std::string& type);
    void opPush(const std::string& op);
    void typePush(const std::string& type);
    void varPush(const std::string& name);
    std::string tempPush(const std::string& type, const std::string& pref = "T");
    void begArgList();
    void endArgList();
    void func_sa();
    void arr_sa();
    void keyword_sa(const std::string &kw);
    void newObj();
    void newArr();
    void ctordecl(const std::string& ctorname);
    void builtin_sa(const std::string& func);

    bool idExist();

    bool typeExist(const std::string &type);

    ////////////////////////////////
    // Operator semantic actions  //
    ////////////////////////////////
    void closeParen();

    void processOperatorStack();

    void closeBracket();
    void end_of_expr();

    /////////////////////////////
    // Code generation helpers //
    /////////////////////////////
    std::string findInScope( const std::string& id );
    std::string getRval(const SARPtr& rval);
    std::string getLval(const SARPtr& lval);
    std::string getRefval(const SARPtr& context, const SARPtr& rval);
    std::string getRefval(const std::string& ctxId, const SARPtr& rval);
    std::string resolveArgument(const SARPtr& arg);

public:
    void setPass(int pass) { this->pass = pass; }
    bool pass1() { return pass == 1; }
    bool pass2() { return pass == 2; }
    ScopeMap getSymbolNameMap() { return symbol_name_map; }
    ScopeMap getSymbolIdMap() { return symbol_id_map; }
    int getLineNumber() { return lexer->getLineNumber(); }
    bool Failed() { return failed; }

protected:
    const std::string getScopeString();

    void raiseError(const std::string& type, const Token& found);

    bool is_in_set( const std::string& needle, const std::set<std::string>& haystack );
    bool is_reserved( const std::string& kw );
    void assert_type(const TokenType type);
    void assert_type_value(const TokenType type, const std::string& value);
    void assert_is( bool param1 );
    std::string makeSymbolId( const std::string& prefix );
    void registerSymbol( const SymbolEntryPtr& symbol );

    template <class T, class T2>
    bool is_a(const T2& in) {
        boost::shared_ptr<T> ptr(boost::dynamic_pointer_cast<T>(in));
        return ptr;
    }

    template <class T, class T2>
    boost::shared_ptr<T> as(const T2& in) {
        return boost::dynamic_pointer_cast<T>(in);
    }

    SARPtr saPop() {
        if (saStack.empty())
            throw SyntaxParserException("Tried to pop an SA off the stack, but none was found");
        SARPtr tmp(saStack.back());
        saStack.pop_back();
        return tmp;
    }
    std::string opPop() {
        std::string tmp(opStack.back());
        opStack.pop_back();
        return tmp;
    }
    std::string getScopeType( const SARPtr& sar );
    std::string getScopeType( const std::string& addr );
    bool compatibleTypes( const SARPtr& sar1, const SARPtr& sar2 );
    bool findFunction( const std::string& scope, const std::string& name, const boost::shared_ptr<argList_SAR>& argList );
    std::string findClass();
    int getTypeSize( const std::string& type, bool nested = false );

protected:
    int lastLine;
    void LineComment();

private:
    LexicalParser* lexer;
    ICodeWriter* icode;
    std::set<std::string> validKeywords;
    std::set<std::string> validTypes;
    std::set<std::string> validModifiers;
    std::set<std::string> validOperators;

    std::deque<SARPtr> saStack;
    std::deque<std::string> opStack;
    std::deque<std::string> current_scope;
    std::deque<std::string> scope_type;
    std::deque<std::string> loop_stack;
    ScopeMap symbol_id_map;
    ScopeMap symbol_name_map;
    bool foundConstructor;
    bool foundReturn;

    std::map<std::string, int> operatorPrecedence;

    long nextId;

    std::string lastSeenName;
    std::string lastSeenFunction;
    std::string lastSeenModifier;
    std::string lastSeenType;
    std::string lastSeenFuncType;
    std::string lastSeenFieldType;

    std::vector<std::string> foundParams;
    int pass;
    bool failed;
    bool mainWritten;
    friend class scoped_PushScope;
};
