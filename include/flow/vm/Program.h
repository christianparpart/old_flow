#pragma once

#include <flow/vm/Instruction.h>

#include <vector>
#include <utility>
#include <memory>

#if 1 == 0 // {{{
void f() {
    FlowVM::Program program;
    program.setStrings({"", "Hello", "World", " ", "!"});
    program.setIntegers({42, 7, 123456789, 56789});
    program.setIPAddresses({"127.0.0.1", "192.168.0.1"});
    program.setCidrs({"127.0.0.0/8", "192.168.0.0/16"});
    program.setRegExps({"^foo.*r$"});
    program.setNativeFunctions({{"rand(II)I", my_rand}, {"print(S)", my_print}});
    program.setNativeHandlers({{"myhnd1", my_hnd1}, {"myhnd2", my_hnd2}});

    FlowVM::Handler main;
    main.setRegisterCount(32);
    main.setCode({
        flow::vm::makeInstructionImm(flow::vm::Opcode::DUMPREGS, 0, 8),
        flow::vm::makeInstruction(flow::vm::Opcode::EXIT, 1),
    });
}
#endif  // }}}

namespace FlowVM {

class Runner;
class Runtime;
class Handler;

class Program
{
public:
    Program();
    Program(
        const std::vector<uint64_t>& constNumbers,
        const std::vector<std::string>& constStrings,
        const std::vector<std::string>& regularExpressions);
    Program(Program&) = delete;
    Program& operator=(Program&) = delete;
    ~Program();

    inline const std::vector<uint64_t>& numbers() const { return numbers_; }
    inline const std::vector<std::string>& strings() const { return strings_; }
    inline const std::vector<std::string>& regularExpressions() const { return regularExpressions_; }
    inline const std::vector<Handler*> handlers() const { return handlers_; }

    Handler* createHandler(const std::string& signature, int registerCount, const std::vector<Instruction>& instructions);
    Handler* findHandler(const std::string& signature) const;

    void link(Runtime* runtime);

private:
    std::vector<uint64_t> numbers_;
    std::vector<std::string> strings_;
    std::vector<std::string> regularExpressions_; // XXX should be a pre-compiled handled during runtime
    std::vector<Handler*> handlers_;
    Runtime* runtime_;
};

} // namespace FlowVM
