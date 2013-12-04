#pragma once

#include <flow/vm/Instruction.h>
#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace FlowVM {

class Program;
class Runner;

class Handler
{
public:
    Handler();
    Handler(Program* program, const std::string& name,
        const std::vector<Instruction>& instructions);
    Handler(const Handler& handler);
    Handler(Handler&& handler);
    ~Handler();

    Program* program() const { return program_; }

    const std::string& name() const { return name_; }
    void setName(const std::string& name) { name_ = name; }

    size_t registerCount() const { return registerCount_; }
    void setRegisterCount(size_t v) { registerCount_ = v; }

    const std::vector<Instruction>& code() const { return code_; }
    void setCode(const std::vector<Instruction>& code) { code_ = code; }
    void setCode(std::vector<Instruction>&& code) { code_ = std::move(code); }

    std::unique_ptr<Runner> createRunner();

    void disassemble();

private:
    Program* program_;
    std::string name_;
    size_t registerCount_;
    std::vector<Instruction> code_;
};

} // namespace FlowVM
