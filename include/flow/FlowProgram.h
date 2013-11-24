#pragma once

#include <flow/FlowInstruction.h>

#include <vector>
#include <utility>
#include <memory>

class FlowRunner;

class FlowProgram
{
public:
    FlowProgram();
    FlowProgram(
        const std::vector<FlowInstruction>& instructions,
        const std::vector<uint64_t>& constNumbers,
        const std::vector<std::string>& constStrings,
        const std::vector<std::string>& regularExpressions,
        size_t numRegisters);
    FlowProgram(FlowProgram&) = delete;
    FlowProgram& operator=(FlowProgram&) = delete;
    ~FlowProgram();

    inline const std::vector<FlowInstruction>& instructions() const;
    inline const std::vector<uint64_t>& numbers() const;
    inline const std::vector<std::string>& strings() const;
    inline const std::vector<std::string>& regularExpressions() const;
    inline size_t registerCount() const;

    std::unique_ptr<FlowRunner> createRunner();

private:
    std::vector<FlowInstruction> instructions_;
    std::vector<uint64_t> numbers_;
    std::vector<std::string> strings_;
    std::vector<std::string> regularExpressions_; // XXX should be a pre-compiled handled during runtime
    size_t registerCount_;
};

// {{{ inlines
inline const std::vector<FlowInstruction>& FlowProgram::instructions() const
{
    return instructions_;
}

inline const std::vector<uint64_t>& FlowProgram::numbers() const
{
    return numbers_;
}

inline const std::vector<std::string>& FlowProgram::strings() const
{
    return strings_;
}

inline const std::vector<std::string>& FlowProgram::regularExpressions() const
{
    return regularExpressions_;
}

inline size_t FlowProgram::registerCount() const
{
    return registerCount_;
}
// }}}
