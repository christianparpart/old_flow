#pragma once

#include <flow/vm/Type.h>
#include <flow/vm/Handler.h>
#include <flow/vm/Instruction.h>
#include <flow/vm/Runtime.h>        // String
#include <utility>
#include <list>
#include <memory>
#include <new>
#include <cstdint>
#include <cstdio>
#include <cmath>
#include <string>

namespace FlowVM {

typedef uint64_t Register;

// ExecutionEngine
// VM
class Runner
{
private:
    Handler* handler_;
    Program* program_;
    void* userdata_;

    std::list<std::string> stringGarbage_;

    Register data_[];

public:
    static std::unique_ptr<Runner> create(Handler* handler);
    static void operator delete (void* p);

    bool run();

    Handler* handler() const { return handler_; }
    Program* program() const { return program_; }
    void* userdata() const { return userdata_; }
    void setUserData(void* p) { userdata_ = p; }

    String* createString(const std::string& value);

private:
    explicit Runner(Handler* handler);
    Runner(Runner&) = delete;
    Runner& operator=(Runner&) = delete;
};

} // namespace FlowVM
