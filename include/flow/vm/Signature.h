#pragma once

#include <flow/vm/Type.h>
#include <vector>
#include <string>

namespace FlowVM {

class Signature {
private:
    std::string name_;
    Type returnType_;
    std::vector<Type> args_;

public:
    Signature();
    Signature(const std::string& signature);
    Signature(const Signature& v) :
        name_(v.name_),
        returnType_(v.returnType_),
        args_(v.args_)
    {}

    void setName(const std::string& name) { name_ = name; }
    void setReturnType(Type rt) { returnType_ = rt; }
    void setArgs(const std::vector<Type>& args) { args_ = args; }
    void setArgs(std::vector<Type>&& args) { args_ = std::move(args); }

    const std::string& name() const { return name_; }
    Type returnType() const { return returnType_; }
    const std::vector<Type>& args() const { return args_; }
    std::vector<Type>& args() { return args_; }

    std::string to_s() const;

    bool operator==(const Signature& v) const { return to_s() == v.to_s(); }
    bool operator!=(const Signature& v) const { return to_s() != v.to_s(); }
    bool operator<(const Signature& v) const { return to_s() < v.to_s(); }
    bool operator>(const Signature& v) const { return to_s() > v.to_s(); }
    bool operator<=(const Signature& v) const { return to_s() <= v.to_s(); }
    bool operator>=(const Signature& v) const { return to_s() >= v.to_s(); }
};

Type typeSignature(char ch);
char signatureType(Type t);

} // namespace FlowVM
