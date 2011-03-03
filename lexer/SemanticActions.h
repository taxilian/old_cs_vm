#ifndef SymanticActions_h__
#define SymanticActions_h__

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

struct SAR
{
    SAR(const std::string& val) : value(val) {}
    std::string value;
};
typedef boost::shared_ptr<SAR> SARPtr;

struct id_SAR : public SAR
{
    id_SAR(const std::string& val) : SAR(val) {}
};

struct type_SAR : public SAR
{
    type_SAR(const std::string& val) : SAR(val) {}
};

struct var_SAR : public SAR
{
    std::string type;
    var_SAR(const std::string& val, const std::string& type) : SAR(val), type(type) {}
};

struct ref_SAR : public SAR
{
    ref_SAR(const std::string& val) : SAR(val) {}
};

struct begArgList_SAR : public SAR
{
    begArgList_SAR(const std::string& val) : SAR(val) {}
};

struct argList_SAR : public SAR
{
    argList_SAR(const std::string& val) : SAR(val) {}
};

struct func_SAR : public SAR
{
    func_SAR(const std::string& val) : SAR(val) {}
};

struct arrRef_SAR : public SAR
{
    arrRef_SAR(const std::string& val) : SAR(val) {}
};

struct int_SAR : public SAR
{
    int_SAR(const std::string& val) : SAR(val) {}
};

struct bool_SAR : public SAR
{
    bool_SAR(const std::string& val) : SAR(val) {}
};

struct null_SAR : public SAR
{
    null_SAR() : SAR("null") {}
};

struct char_SAR : public SAR
{
    char_SAR(const std::string& val) : SAR(val) {}
};

struct new_SAR : public SAR
{
    new_SAR(const std::string& val) : SAR(val) {}
};

struct tvar_SAR : public SAR
{
    std::string type;
    tvar_SAR(const std::string& val, const std::string& type) : SAR(val), type(type) {}
};
#endif // SymanticActions_h__