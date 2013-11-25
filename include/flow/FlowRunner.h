#pragma once

#include <flow/FlowProgram.h>
#include <flow/FlowInstruction.h>
#include <utility>
#include <list>
#include <memory>
#include <new>
#include <cstdint>
#include <cstdio>
#include <cmath>
#include <string>

typedef std::string FlowString;
typedef int64_t FlowNumber;

typedef uint64_t FlowRegister;

class FlowRunner
{
private:
    FlowProgram* program_;
    void* userdata_;

    std::list<std::string> stringGarbage_;

    FlowRegister data_[];

public:
    static std::unique_ptr<FlowRunner> create(FlowProgram* program);
    static void operator delete (void* p);

    bool run();

    FlowProgram* program() const { return program_; }
    void* userdata() const { return userdata_; }
    void setUserData(void* p) { userdata_ = p; }

private:
    explicit FlowRunner(FlowProgram* program);
    FlowRunner(FlowRunner&) = delete;
    FlowRunner& operator=(FlowRunner&) = delete;
};
