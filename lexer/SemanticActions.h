#ifndef SymanticActions_h__
#define SymanticActions_h__

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

struct SAR
{
    SAR(const std::string& val) : value(val) {}
    virtual ~SAR() {}
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
    argList_SAR() : SAR("") {}

    void pushArg(const SARPtr& arg) {
        argList.push_back(arg);
    }
    std::vector<SARPtr> argList;
};

struct func_SAR : public SAR
{
    func_SAR(const std::string& val) : SAR(val) {}
};

struct arrRef_SAR : public SAR
{
    arrRef_SAR(const std::string& val) : SAR(val) {}
};

struct lit_SAR : public SAR
{
    std::string type;
    lit_SAR(const std::string& val, const std::string& type) : SAR(val), type(type) {}
};

struct int_SAR : public lit_SAR
{
    int_SAR(const std::string& val) : lit_SAR(val, "int") {}
};

struct bool_SAR : public lit_SAR
{
    bool_SAR(const std::string& val) : lit_SAR(val, "bool") {}
};

struct null_SAR : public lit_SAR
{
    null_SAR() : lit_SAR("null", "null") {}
};

struct char_SAR : public lit_SAR
{
    char_SAR(const std::string& val) : lit_SAR(val, "char") {}
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