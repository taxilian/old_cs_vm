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

struct typedSAR : public SAR
{
    typedSAR(const std::string& val, const std::string& type) : SAR(val), type(type) {}

    std::string type;
    virtual std::string getType() { return type; }
};

struct id_SAR : public SAR
{
    id_SAR(const std::string& val) : SAR(val) {}
};

struct type_SAR : public typedSAR
{
    type_SAR(const std::string& val) : typedSAR(val, val) {}
};

struct var_SAR : public typedSAR
{
    var_SAR(const std::string& val, const std::string& type) : typedSAR(val, type) {}
};

struct ref_SAR : public typedSAR
{
    ref_SAR(const std::string& val, const std::string& type) : typedSAR(val, type) {}
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

struct func_SAR : public typedSAR
{
    func_SAR(const std::string& val, const std::string& type) : typedSAR(val, type) {}
    std::vector<SARPtr> argList;
};

struct arrRef_SAR : public SAR
{
    arrRef_SAR(const std::string& val) : SAR(val) {}
};

struct lit_SAR : public typedSAR
{
    lit_SAR(const std::string& val, const std::string& type) : typedSAR(val, type) {}
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

struct void_SAR : public lit_SAR
{
    void_SAR() : lit_SAR("void", "void") {}
};

struct new_SAR : public func_SAR
{
    new_SAR(const std::string& val, const std::string& type) : func_SAR(val, type) {}
};

struct tvar_SAR : public typedSAR
{
    tvar_SAR(const std::string& val, const std::string& type) : typedSAR(val, type) {}
};
#endif // SymanticActions_h__
