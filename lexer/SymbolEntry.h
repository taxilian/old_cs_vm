#pragma once
#ifndef SymbolEntry_h__
#define SymbolEntry_h__


#include <string>
#include <list>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

struct SymbolData : boost::noncopyable {
    std::string accessMod;
};
typedef boost::shared_ptr<SymbolData> SymbolDataPtr;

struct ParameterDef : boost::noncopyable {
    std::string paramId;
};
typedef boost::shared_ptr<ParameterDef> ParameterDefPtr;

struct MethodData : public SymbolData {
    std::list<ParameterDefPtr> Parameters;
    std::string returnType;
};
typedef boost::shared_ptr<MethodData> MethodDataPtr;

struct TypeData : public SymbolData {
    std::string type;
};
typedef boost::shared_ptr<TypeData> TypeDataPtr;

struct SymbolEntry : boost::noncopyable {
    std::string scope;
    std::string id;
    std::string value;
    std::string kind;
    SymbolDataPtr data;
};
typedef boost::shared_ptr<SymbolEntry> SymbolEntryPtr;
#endif // SymbolEntry_h__