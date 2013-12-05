#pragma once

#include <flow/vm/Instruction.h>
#include <flow/vm/Runtime.h>        // Runtime::Callback
#include <flow/vm/Type.h>           // Number

#include <vector>
#include <utility>
#include <memory>

namespace FlowVM {

class Runner;
class Handler;

class Program
{
public:
    Program();
    Program(
        const std::vector<Number>& constNumbers,
        const std::vector<std::string>& constStrings,
        const std::vector<std::string>& regularExpressions,
        const std::vector<std::pair<std::string, std::string>>& modules,
        const std::vector<std::string>& nativeHandlerSignatures,
        const std::vector<std::string>& nativeFunctionSignatures
    );
    Program(Program&) = delete;
    Program& operator=(Program&) = delete;
    ~Program();

    inline const std::vector<Number>& numbers() const { return numbers_; }
    inline const std::vector<String>& strings() const { return strings_; }
    inline const std::vector<std::string>& regularExpressions() const { return regularExpressions_; }
    inline const std::vector<Handler*> handlers() const { return handlers_; }

    Handler* createHandler(const std::string& name);
    Handler* createHandler(const std::string& name, const std::vector<Instruction>& instructions);
    Handler* findHandler(const std::string& name) const;
    Handler* handler(size_t index) const { return handlers_[index]; }
    int handlerIndex(const std::string& name) const;

    Runtime::Callback* nativeHandler(size_t id) const { return nativeHandlers_[id]; }
    Runtime::Callback* nativeFunction(size_t id) const { return nativeFunctions_[id]; }

    bool link(Runtime* runtime);

    void dump();

private:
    std::vector<Number> numbers_;
    std::vector<String> strings_;
    std::vector<std::string> regularExpressions_;               // XXX to be a pre-compiled handled during runtime
    std::vector<std::pair<std::string, std::string>> modules_;
    std::vector<std::string> nativeHandlerSignatures_;
    std::vector<std::string> nativeFunctionSignatures_;

    std::vector<Runtime::Callback*> nativeHandlers_;
    std::vector<Runtime::Callback*> nativeFunctions_;
    std::vector<Handler*> handlers_;
    Runtime* runtime_;
};

} // namespace FlowVM
