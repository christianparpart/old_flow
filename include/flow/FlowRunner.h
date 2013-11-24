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
    FlowProgram* program;
    void* userdata;
    FlowRegister data[];

public:
    bool run();

    static std::unique_ptr<FlowRunner> create(FlowProgram* program);
    static void operator delete (void* p);

private:
    FlowRunner(FlowProgram* _program) : program(_program), userdata(nullptr) { }
};
