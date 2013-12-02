#include <flow/vm/Program.h>
#include <flow/vm/Handler.h>
#include <flow/vm/Instruction.h>
#include <flow/vm/Runner.h>
#include <utility>
#include <vector>
#include <memory>
#include <new>

namespace FlowVM {

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

Program::Program() :
    numbers_(),
    strings_(),
    regularExpressions_(),
    handlers_(),
    runtime_(nullptr)
{
}

Program::Program(
        const std::vector<uint64_t>& numbers,
        const std::vector<std::string>& strings,
        const std::vector<std::string>& regularExpressions) :
    numbers_(numbers),
    strings_(strings),
    regularExpressions_(regularExpressions),
    handlers_(),
    runtime_(nullptr)
{
}

Program::~Program()
{
    for (auto& handler: handlers_)
        delete handler;
}

Handler* Program::createHandler(const std::string& signature, int registerCount, const std::vector<Instruction>& instructions)
{
    Handler* handler = new Handler(this, signature, registerCount, instructions);
    handlers_.push_back(handler);

    return handler;
}

Handler* Program::findHandler(const std::string& signature) const
{
    for (auto handler: handlers_)
        if (handler->signature() == signature)
            return handler;

    return nullptr;
}

void Program::link(Runtime* runtime)
{
    runtime_ = runtime;
    // TODO: verify that all functions do exist
}

} // namespace FlowVM
