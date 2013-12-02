#pragma once

#include <flow/vm/Type.h>
#include <string>
#include <vector>
#include <functional>

namespace FlowVM {

typedef uint64_t Value;

class Runner;

typedef std::function<void(int argc, Value* argv, Runner* cx)> NativeCallback;

class Runtime
{
public:
    struct Callback { // {{{
        Runtime* runtime_;
        bool isHandler_;
        std::string name_;
        NativeCallback function_;
        std::vector<Type> signature_;

        bool isHandler() const { return isHandler_; }
        const std::string name() const { return name_; }
        const std::vector<Type>& signature() const { return signature_; }

        // constructs a handler callback
        Callback(Runtime* runtime, const std::string& _name) :
            runtime_(runtime),
            isHandler_(true),
            name_(_name),
            function_(),
            signature_()
        {
            signature_.push_back(Type::Boolean);
        }

        // constructs a function callback
        Callback(Runtime* runtime, const std::string& _name, Type _returnType) :
            runtime_(runtime),
            isHandler_(false),
            name_(_name),
            function_(),
            signature_()
        {
            signature_.push_back(_returnType);
        }

        Callback(const std::string& _name, const NativeCallback& _builtin, Type _returnType) :
            isHandler_(false),
            name_(_name),
            function_(_builtin),
            signature_()
        {
            signature_.push_back(_returnType);
        }

        void invoke(int argc, Value* argv, Runner* cx) const {
            function_(argc, argv, cx);
        }

        template<typename Arg1>
        Callback& signature(Arg1 a1) {
            signature_.push_back(a1);
            return *this;
        }

        template<typename Arg1, typename... Args>
        Callback& signature(Arg1 a1, Args... more) {
            signature_.push_back(a1);
            return signature(more...);
        }

        Callback& operator()(const NativeCallback& cb) {
            function_ = cb;
            return *this;
        }

        Callback& bind(const NativeCallback& cb) {
            function_ = cb;
            return *this;
        }

        template<typename Class>
        Callback& bind(void (Class::*method)(int, Value*, Runner*), Class* obj) {
            function_ = std::bind(method, obj,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

            return *this;
        }

        template<typename Class>
        Callback& bind(void (Class::*method)(int, Value*, Runner*)) {
            function_ = std::bind(method, static_cast<Class*>(runtime_),
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

            return *this;
        }
    }; // }}}
public:
	virtual bool import(const std::string& name, const std::string& path) = 0;

	bool contains(const std::string& name) const;
	int find(const std::string& name) const;
	const std::vector<Callback>& builtins() const { return builtins_; }

	Callback& registerHandler(const std::string& name);
	Callback& registerFunction(const std::string& name, Type returnType);

//	template<typename... Args>
//	Callback& registerFunction(const std::string& name, const NativeCallback& fn, Type returnType, Args... args);

	void invoke(int id, int argc, Value* argv, Runner* cx);

private:
    std::vector<Callback> builtins_;
};

}
