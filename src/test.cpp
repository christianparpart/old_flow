#include <flow/vm/Program.h>
#include <flow/vm/Runner.h>
#include <flow/vm/Instruction.h>
#include <flow/vm/Runtime.h>
#include <initializer_list>
#include <vector>
#include <utility>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <memory>
#include <new>

#include <unistd.h> // getcwd()

namespace FlowVM {

class Signature {
private:
    std::string name_;
    Type returnType_;
    std::vector<Type> args_;

public:
    Signature(const std::string& signature);
    Signature(const Signature& v) :
        name_(v.name_),
        returnType_(v.returnType_),
        args_(v.args_)
    {}

    const std::string& name() const { return name_; }
    Type returnType() const { return returnType_; }
    const std::vector<Type>& args() const { return args_; }

    std::string to_s() const;

    bool operator==(const Signature& v) const { return to_s() == v.to_s(); }
    bool operator!=(const Signature& v) const { return to_s() != v.to_s(); }
    bool operator<(const Signature& v) const { return to_s() < v.to_s(); }
};

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
        case Type::StringArray: return 'A';//XXX
        default: return '?';
    }
}

Signature::Signature(const std::string& signature)
{
    // signature  ::= NAME [ '(' args ')' returnType
    // args       ::= type*
    // returnType ::= type
    // type       ::= B | I | S | P | C | H
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
                printf("Garbage at end of signature string. %s\n", i);
                break;
        }
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

} // namespace FlowVM

static const std::vector<FlowVM::Instruction> code1 = {
    makeInstructionImm(FlowVM::Opcode::EXIT, 1),
};

/*
 * r1 = 0;
 * r2 = 0;
 *
 * while (r1 < 4) {
 *     r1 = r1 + 1;
 *     r2 = r2 + r1;
 * }
 *
 */
static const std::vector<FlowVM::Instruction> code2 = {
    // prolog
    makeInstructionImm(FlowVM::Opcode::IMOV, 0, 4),     // r0 = 4
    makeInstructionImm(FlowVM::Opcode::IMOV, 1, 0),     // r1 = 0
    makeInstructionImm(FlowVM::Opcode::IMOV, 2, 0),     // r2 = 0
    makeInstructionImm(FlowVM::Opcode::IMOV, 4, 1),     // r4 = 1
    makeInstructionImm(FlowVM::Opcode::JMP, 7),         // IP = condition

    // loop body
    makeInstruction(FlowVM::Opcode::NADD, 1, 1, 4),     // r1 = r1 + 1
    makeInstruction(FlowVM::Opcode::NADD, 2, 2, 1),     // r2 = r2 + r1

    // condition
    makeInstruction(FlowVM::Opcode::NCMPLT, 3, 1, 0),   // r3 = r1 < r0 ; 4
    makeInstructionImm(FlowVM::Opcode::CONDBR, 3, 5),   // if isTrue(r3) then IP = loopBody

    // epilog
    makeInstruction(FlowVM::Opcode::NDUMPN, 0, 5),

    makeInstruction(FlowVM::Opcode::NCONST, 0, 0),      // r0 = nconst[0]
    makeInstruction(FlowVM::Opcode::NCONST, 1, 1),      // r1 = nconst[1]
    makeInstruction(FlowVM::Opcode::NSUB, 2, 0, 1),     // r2 = r0 - r1
    makeInstruction(FlowVM::Opcode::NDUMPN, 0, 3),

    makeInstructionImm(FlowVM::Opcode::EXIT, 1),
};

/*
 * r0 = " "
 * r1 = "Hello"
 * r2 = "World"
 * r3 = r1 + r2
 * sdump r3
 *
 */
static const std::vector<FlowVM::Instruction> code3 = {
    makeInstructionImm(FlowVM::Opcode::SCONST, 0, 0),   // r1 = sconst[0]
    makeInstructionImm(FlowVM::Opcode::SCONST, 1, 1),   // r1 = sconst[1]
    makeInstructionImm(FlowVM::Opcode::SCONST, 2, 2),   // r2 = sconst[2]
    makeInstruction(FlowVM::Opcode::SADD, 3, 1, 0),
    makeInstruction(FlowVM::Opcode::SADD, 3, 3, 2),
    makeInstruction(FlowVM::Opcode::SPRINT, 3),

    makeInstructionImm(FlowVM::Opcode::IMOV, 5, 1),
    makeInstructionImm(FlowVM::Opcode::IMOV, 6, 9),
    makeInstruction(FlowVM::Opcode::SSUBSTR, 4, 3, 5),
    makeInstruction(FlowVM::Opcode::SPRINT, 4),
    makeInstruction(FlowVM::Opcode::NDUMPN, 0, 7),

    makeInstructionImm(FlowVM::Opcode::SCONST, 7, 4),   // r7 = sconst[4] /* "rl" */
    makeInstruction(FlowVM::Opcode::SCONTAINS, 8, 3, 4),
    makeInstruction(FlowVM::Opcode::NDUMPN, 7, 2),

    makeInstructionImm(FlowVM::Opcode::EXIT, 1),
};

class FlowTest : public FlowVM::Runtime {
public:
    FlowTest()
    {
        registerHandler("assert")
            .signature(FlowVM::Type::Boolean, FlowVM::Type::String)
            .bind(&FlowTest::_assert);

        registerFunction("getcwd", FlowVM::Type::String)
            .bind(&FlowTest::_getcwd);

        registerFunction("print", FlowVM::Type::Number)
            .signature(FlowVM::Type::StringArray)
            .bind(&FlowTest::_print);
    }

    virtual bool import(const std::string& name, const std::string& path)
    {
        return false;
    }

    // signature:
    //     "assert(BS)B"
    //      bool assert(bool exprResult, string exprSourceCode);
    void _assert(int argc, FlowVM::Value* argv, FlowVM::Runner* cx)
    {
        printf("_assert!\n");
        argv[0] = false;
//        printf("assertion: %s   ; %s\n",
//            args[1].toBool() ? "true" : "false",
//            args[1].toString());

//        args[0] = args[1].toBool();
    }

    void _print(int argc, FlowVM::Value* argv, FlowVM::Runner* cx)
    {
        printf("_print!\n");
    }

    void _getcwd(int argc, FlowVM::Value* argv, FlowVM::Runner* cx)
    {
        printf("_getcwd!\n");

        char cwd[PATH_MAX];
        getcwd(cwd, sizeof(cwd));
        argv[0] = (FlowVM::Value) cx->createString(cwd);
    }
};

int main()
{
    using FlowVM::Signature;

    Signature sig("assert(BSI)B");
    std::printf("sig: %s\n", sig.to_s().c_str());
    exit(0);

    FlowVM::Program program(
        {123456789, 56789},                 // integer constants
        {"", "Hello", "World", " ", "rl"},  // string constants
        {"^H.ll. W.rld$"},                  // regex constants
        {"assert(BS)B"},                    // native handler signatures
        {"print(S)V", "getcwd()S"}          // native function signatures
    );

    program.createHandler("test1", code1);
    program.createHandler("test2", code2);
    program.createHandler("test3", code3);

    FlowTest runtime;
    program.link(&runtime);

    if (FlowVM::Handler* handler = program.findHandler("test3")) {
        printf("Disassembling %s ...\n", handler->signature().c_str());
        handler->disassemble();

        printf("\nRunning %s ...\n", handler->signature().c_str());
        std::unique_ptr<FlowVM::Runner> flow = handler->createRunner();
        flow->run();
    }

    return 0;
}
