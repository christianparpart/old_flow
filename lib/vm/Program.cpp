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
    modules_(),
    nativeHandlerSignatures_(),
    nativeFunctionSignatures_(),
    nativeHandlers_(),
    nativeFunctions_(),
    handlers_(),
    runtime_(nullptr)
{
}

Program::Program(
        const std::vector<Number>& numbers,
        const std::vector<String>& strings,
        const std::vector<std::string>& regularExpressions,
        const std::vector<std::pair<std::string, std::string>>& modules,
        const std::vector<std::string>& nativeHandlerSignatures,
        const std::vector<std::string>& nativeFunctionSignatures) :
    numbers_(numbers),
    strings_(strings),
    regularExpressions_(regularExpressions),
    modules_(modules),
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

Handler* Program::createHandler(const std::string& name, const std::vector<Instruction>& instructions)
{
    Handler* handler = new Handler(this, name, instructions);
    handlers_.push_back(handler);

    return handler;
}

Handler* Program::findHandler(const std::string& name) const
{
    for (auto handler: handlers_)
        if (handler->name() == name)
            return handler;

    return nullptr;
}

void Program::dump()
{
    printf("; Program\n");

    printf("\n; Modules\n");
    for (size_t i = 0, e = modules_.size(); i != e; ++i) {
        if (modules_[i].second.empty())
            printf("  \"%s\"\n", modules_[i].first.c_str());
        else
            printf("  \"%s\" from \"%s\"\n", modules_[i].first.c_str(), modules_[i].second.c_str());
    }

    printf("\n; External Functions\n");
    for (size_t i = 0, e = nativeFunctionSignatures_.size(); i != e; ++i) {
        if (nativeFunctions_[i])
            printf("  #%-4zu = %-20s ; linked to %p\n", i, nativeFunctionSignatures_[i].c_str(), nativeFunctions_[i]);
        else
            printf("  #%-4zu = %-20s\n", i, nativeFunctionSignatures_[i].c_str());
    }

    printf("\n; External Handlers\n");
    for (size_t i = 0, e = nativeHandlerSignatures_.size(); i != e; ++i) {
        if (nativeHandlers_[i])
            printf("  #%-4zu = %-20s ; linked to %p\n", i, nativeHandlerSignatures_[i].c_str(), nativeHandlers_[i]);
        else
            printf("  #%-4zu = %-20s\n", i, nativeHandlerSignatures_[i].c_str());
    }

    printf("\n; Integer Constants\n");
    for (size_t i = 0, e = numbers_.size(); i != e; ++i) {
        printf("  #%-4zu = %li\n", i, (Number) numbers_[i]);
    }

    printf("\n; String Constants\n");
    for (size_t i = 0, e = strings_.size(); i != e; ++i) {
        printf("  #%-4zu = \"%s\"\n", i, strings_[i].c_str());
    }

    for (size_t i = 0, e = handlers_.size(); i != e; ++i) {
        Handler* handler = handlers_[i];
        printf("\n; Handler #%zu %s (%zu registers)\n", i, handler->name().c_str(), handler->registerCount());
        handler->disassemble();
    }

    printf("\n\n");
}

/**
 * Maps all native functions/handlers to their implementations (report unresolved symbols)
 *
 * \param runtime the runtime to link this program against, resolving any external native symbols.
 * \retval true Linking succeed.
 * \retval false Linking failed due to unresolved native signatures not found in the runtime.
 */
bool Program::link(Runtime* runtime)
{
    runtime_ = runtime;
    int errors = 0;

    // load runtime modules
    for (const auto& module: modules_) {
        if (!runtime->import(module.first, module.second)) {
            errors++;
        }
    }

    // link nattive handlers
    nativeHandlers_.resize(nativeHandlerSignatures_.size());
    size_t i = 0;
    for (const auto& signature: nativeHandlerSignatures_) {
        // map to nativeHandlers_[i]
        if (Runtime::Callback* cb = runtime->find(signature)) {
            nativeHandlers_[i] = cb;
        } else {
            nativeHandlers_[i] = nullptr;
            fprintf(stderr, "Unresolved native handler signature: %s\n", signature.c_str());
            // TODO unresolvedSymbols_.push_back(signature);
            errors++;
        }
        ++i;
    }

    // link nattive functions
    nativeFunctions_.resize(nativeFunctionSignatures_.size());
    i = 0;
    for (const auto& signature: nativeFunctionSignatures_) {
        if (Runtime::Callback* cb = runtime->find(signature)) {
            nativeFunctions_[i] = cb;
        } else {
            nativeFunctions_[i] = nullptr;
            fprintf(stderr, "Unresolved native function signature: %s\n", signature.c_str());
            // TODO unresolvedSymbols_.push_back(signature);
            errors++;
        }
        ++i;
    }

    return errors == 0;
}

} // namespace FlowVM
