#pragma once
#include "LexicalParser.h"
#include <set>
#include <string>
#include <deque>
#include <map>
#include "SymbolEntry.h"

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

class SyntaxParser
{
public:
    SyntaxParser(LexicalParser& lexer);
    ~SyntaxParser(void);

    // Names, Types, Literals
    void modifier();

    void class_name();
    void type();
    void character_literal();
    void numeric_literal();
    void number();
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
private:
    LexicalParser& lexer;
    std::set<std::string> validKeywords;
    std::set<std::string> validTypes;
    std::set<std::string> validModifiers;
    std::set<std::string> validOperators;

    std::deque<std::string> current_scope;
    std::deque<std::string> scope_type;
    std::map<std::string, SymbolEntryPtr> symbol_id_map;
    std::map<std::string, SymbolEntryPtr> symbol_name_map;

    long nextId;

    std::string lastSeenName;
    std::string lastSeenModifier;
    std::string lastSeenType;
    std::string lastSeenFieldType;

    std::list<ParameterDefPtr> foundParams;
};

