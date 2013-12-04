#include <flow/vm/Signature.h>
#include <vector>
#include <string>

namespace FlowVM {

Signature::Signature() :
    name_(),
    returnType_(Type::Void),
    args_()
{
}

Signature::Signature(const std::string& signature) :
    name_(),
    returnType_(Type::Void),
    args_()
{
    // signature  ::= NAME [ '(' args ')' returnType
    // args       ::= type*
    // returnType ::= primitive | 'V'
    // type       ::= array | assocArray | primitive
    // array      ::= '[' primitive
    // assocArray ::= '>' primitive primitive
    // primitive  ::= 'B' | 'I' | 'S' | 'P' | 'C' | 'R' | 'H'

    enum class State {
        END         = 0,
        Name        = 1,
        ArgsBegin   = 2,
        Args        = 3,
        ReturnType  = 4
    };

    const char* i = signature.data();
    const char* e = signature.data() + signature.size();
    State state = State::Name;

    while (i != e) {
        switch (state) {
            case State::Name:
                if (*i == '(') {
                    state = State::ArgsBegin;
                }
                ++i;
                break;
            case State::ArgsBegin:
                name_ = std::string(signature.data(), i - signature.data() - 1);
                state = State::Args;
                break;
            case State::Args:
                if (*i == ')') {
                    state = State::ReturnType;
                } else {
                    args_.push_back(typeSignature(*i));
                }
                ++i;
                break;
            case State::ReturnType:
                returnType_ = typeSignature(*i);
                state = State::END;
                ++i;
                break;
            case State::END:
                fprintf(stderr, "Garbage at end of signature string. %s\n", i);
                i = e;
                break;
        }
    }

    if (state != State::END) {
        fprintf(stderr, "Premature end of signature string. %s\n", signature.c_str());
    }
}

std::string Signature::to_s() const
{
    std::string result = name_;
    result += "(";
    for (Type t: args_)
        result += signatureType(t);
    result += ")";
    result += signatureType(returnType_);
    return result;
}

Type typeSignature(char ch)
{
    switch (ch) {
        case 'V': return Type::Void;
        case 'B': return Type::Boolean;
        case 'I': return Type::Number;
        case 'S': return Type::String;
        case 'P': return Type::IPAddress;
        case 'C': return Type::Cidr;
        case 'R': return Type::RegExp;
        case 'H': return Type::Handler;
        case '[': return Type::Array;
        case '>': return Type::AssocArray;
        default: return Type::Void; //XXX
    }
}

char signatureType(Type t)
{
    switch (t) {
        case Type::Void: return 'V';
        case Type::Boolean: return 'B';
        case Type::Number: return 'I';
        case Type::String: return 'S';
        case Type::IPAddress: return 'P';
        case Type::Cidr: return 'C';
        case Type::RegExp: return 'R';
        case Type::Handler: return 'H';
        case Type::Array: return '[';
        case Type::AssocArray: return '>';
        default: return '?';
    }
}

} // namespace FlowVM
