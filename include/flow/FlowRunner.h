#pragma once
#include <flow/FlowProgram.h>
#include <flow/FlowInstruction.h>
#include <utility>
#include <memory>
#include <new>
#include <cstdint>
#include <cstdio>
#include <cmath>

typedef uint64_t FlowRegister;

class FlowRunner
{
private:
    FlowProgram* program_;
    void* userdata_;
    FlowRegister data_[];

public:
    static std::unique_ptr<FlowRunner> create(FlowProgram* program);
    static void operator delete (void* p);

    bool run();

    FlowProgram* program() const { return program_; }
    void* userdata() const { return userdata_; }
    void setUserData(void* p) { userdata_ = p; }

private:
    FlowRunner(FlowProgram* program) : program_(program), userdata_(nullptr) { }
};
