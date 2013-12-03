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
    nativeHandlerSignatures_(),
    nativeFunctionSignatures_(),
    nativeHandlers_(),
    nativeFunctions_(),
    handlers_(),
    runtime_(nullptr)
{
}

Program::Program(
        const std::vector<uint64_t>& numbers,
        const std::vector<std::string>& strings,
        const std::vector<std::string>& regularExpressions,
        const std::vector<std::string>& nativeHandlerSignatures,
        const std::vector<std::string>& nativeFunctionSignatures) :
    numbers_(numbers),
    strings_(strings),
    regularExpressions_(regularExpressions),
    nativeHandlerSignatures_(nativeHandlerSignatures),
    nativeFunctionSignatures_(nativeFunctionSignatures),
    nativeHandlers_(),
    nativeFunctions_(),
    handlers_(),
    runtime_(nullptr)
{
}

Program::~Program()
{
    for (auto& handler: handlers_)
        delete handler;
}

Handler* Program::createHandler(const std::string& signature, const std::vector<Instruction>& instructions)
{
    Handler* handler = new Handler(this, signature, instructions);
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

bool Program::link(Runtime* runtime)
{
    runtime_ = runtime;
    // map all native functions/handlers to their implementations (report unresolved symbols)

    size_t i = 0;
    for (const auto& signature: nativeHandlerSignatures_) {
        // map to nativeHandlers_[i]
        ++i;
    }

    i = 0;
    for (const auto& signature: nativeFunctionSignatures_) {
        // map to nativeFunctions_[i]
        ++i;
    }

    return true;
}

} // namespace FlowVM
