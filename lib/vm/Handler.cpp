#include <flow/vm/Handler.h>
#include <flow/vm/Runner.h>
#include <flow/vm/Instruction.h>

namespace FlowVM {

Handler::Handler()
{
}

Handler::Handler(Program* program, const std::string& signature,
        const std::vector<Instruction>& code) :
    program_(program),
    signature_(signature),
    registerCount_(computeRegisterCount(code.data(), code.size())),
    code_(code)
{
    printf("Handler(%s) regcount=%zu\n", signature.c_str(), registerCount_);
}

Handler::Handler(const Handler& v) :
    program_(v.program_),
    signature_(v.signature_),
    registerCount_(v.registerCount_),
    code_(v.code_)
{
}

Handler::Handler(Handler&& v) :
    program_(std::move(v.program_)),
    signature_(std::move(v.signature_)),
    registerCount_(std::move(v.registerCount_)),
    code_(std::move(v.code_))
{
}

Handler::~Handler()
{
}

std::unique_ptr<Runner> Handler::createRunner()
{
    return Runner::create(this);
}

void Handler::disassemble()
{
    FlowVM::disassemble(code_.data(), code_.size());
}

} // namespace FlowVM
