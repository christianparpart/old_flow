#include <flow/FlowInstruction.h>
#include <flow/FlowProgram.h>
#include <flow/FlowRunner.h>
#include <utility>
#include <vector>
#include <memory>
#include <new>

/* {{{ possible binary file format
 * ----------------------------------------------
 * u32                  magic number (0xbeafbabe)
 * u32                  version
 * u64                  flags
 * u64                  register count
 * u64                  code start
 * u64                  code size
 * u64                  integer const-table start
 * u64                  integer const-table element count
 * u64                  string const-table start
 * u64                  string const-table element count
 * u64                  regex const-table start (stored as string)
 * u64                  regex const-table element count
 * u64                  debug source-lines-table start
 * u64                  debug source-lines-table element count
 *
 * u32[]                code segment
 * u64[]                integer const-table segment
 * u64[]                string const-table segment
 * {u32, u8[]}[]        strings
 * {u32, u32, u32}[]    debug source lines segment
 */ // }}}

FlowProgram::FlowProgram() :
    instructions_(),
    numbers_(),
    strings_(),
    regularExpressions_(),
    registerCount_(0)
{
}

FlowProgram::FlowProgram(
        const std::vector<FlowInstruction>& instructions,
        const std::vector<uint64_t>& numbers,
        const std::vector<std::string>& strings,
        const std::vector<std::string>& regularExpressions,
        size_t numRegisters) :
    instructions_(instructions),
    numbers_(numbers),
    strings_(strings),
    regularExpressions_(regularExpressions),
    registerCount_(numRegisters)
{
}

FlowProgram::~FlowProgram()
{
}

std::unique_ptr<FlowRunner> FlowProgram::createRunner()
{
    return FlowRunner::create(this);
}
