#include <flow/vm/Runtime.h>

namespace FlowVM {

Runtime::Callback& Runtime::registerHandler(const std::string& name)
{
    builtins_.push_back(Callback(this, name, Type::Boolean));
    return builtins_[builtins_.size() - 1];
}

Runtime::Callback& Runtime::registerFunction(const std::string& name, Type returnType)
{
    builtins_.push_back(Callback(this, name, returnType));
    return builtins_[builtins_.size() - 1];
}

Runtime::Callback* Runtime::find(const std::string& signature)
{
    for (auto& callback: builtins_)
        if (callback.signature().to_s() == signature)
            return &callback;

    return nullptr;
}

} // namespace FlowVM
