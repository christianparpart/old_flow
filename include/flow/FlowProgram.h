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
        const std::vector<std::pair<char*, size_t>>& constStrings,
        size_t numRegisters);
    ~FlowProgram();

    inline const std::vector<FlowInstruction>& instructions() const;
    inline const std::vector<uint64_t>& numbers() const;
    inline const std::vector<std::pair<char*, size_t>>& strings() const;
    inline size_t registerCount() const;

    std::unique_ptr<FlowRunner> createRunner();

private:
    std::vector<FlowInstruction> instructions_;
    std::vector<uint64_t> numbers_;
    std::vector<std::pair<char*, size_t>> strings_;
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

inline const std::vector<std::pair<char*, size_t>>& FlowProgram::strings() const
{
    return strings_;
}

inline size_t FlowProgram::registerCount() const
{
    return registerCount_;
}
// }}}
