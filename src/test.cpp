#include <flow/vm/Program.h>
#include <flow/vm/Runner.h>
#include <flow/vm/Runtime.h>
#include <flow/vm/Signature.h>
#include <flow/vm/Instruction.h>
#include <initializer_list>
#include <vector>
#include <utility>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <memory>
#include <new>

#include <unistd.h> // getcwd()

static const std::vector<FlowVM::Instruction> code1 = {
    makeInstructionImm(FlowVM::Opcode::EXIT, 1),
};

/*
 * r1 = 0;
 * r2 = 0;
 *
 * while (r1 < 4) {
 *     r1 = r1 + 1;
 *     r2 = r2 + r1;
 * }
 *
 */
static const std::vector<FlowVM::Instruction> code2 = {
    // prolog
    makeInstructionImm(FlowVM::Opcode::IMOV, 0, 4),     // r0 = 4
    makeInstructionImm(FlowVM::Opcode::IMOV, 1, 0),     // r1 = 0
    makeInstructionImm(FlowVM::Opcode::IMOV, 2, 0),     // r2 = 0
    makeInstructionImm(FlowVM::Opcode::IMOV, 4, 1),     // r4 = 1
    makeInstructionImm(FlowVM::Opcode::JMP, 7),         // IP = condition

    // loop body
    makeInstruction(FlowVM::Opcode::NADD, 1, 1, 4),     // r1 = r1 + 1
    makeInstruction(FlowVM::Opcode::NADD, 2, 2, 1),     // r2 = r2 + r1

    // condition
    makeInstruction(FlowVM::Opcode::NCMPLT, 3, 1, 0),   // r3 = r1 < r0 ; 4
    makeInstructionImm(FlowVM::Opcode::CONDBR, 3, 5),   // if isTrue(r3) then IP = loopBody

    // epilog
    makeInstruction(FlowVM::Opcode::NDUMPN, 0, 5),

    makeInstruction(FlowVM::Opcode::NCONST, 0, 0),      // r0 = nconst[0]
    makeInstruction(FlowVM::Opcode::NCONST, 1, 1),      // r1 = nconst[1]
    makeInstruction(FlowVM::Opcode::NSUB, 2, 0, 1),     // r2 = r0 - r1
    makeInstruction(FlowVM::Opcode::NDUMPN, 0, 3),

    makeInstructionImm(FlowVM::Opcode::EXIT, 1),
};

/*
 * r0 = " "
 * r1 = "Hello"
 * r2 = "World"
 * r3 = r1 + r2
 * sdump r3
 *
 */
static const std::vector<FlowVM::Instruction> code3 = {
    makeInstructionImm(FlowVM::Opcode::SCONST, 0, 0),   // r1 = sconst[0]
    makeInstructionImm(FlowVM::Opcode::SCONST, 1, 1),   // r1 = sconst[1]
    makeInstructionImm(FlowVM::Opcode::SCONST, 2, 2),   // r2 = sconst[2]
    makeInstruction(FlowVM::Opcode::SADD, 3, 1, 0),
    makeInstruction(FlowVM::Opcode::SADD, 3, 3, 2),
    makeInstruction(FlowVM::Opcode::SPRINT, 3),

    makeInstructionImm(FlowVM::Opcode::IMOV, 5, 1),
    makeInstructionImm(FlowVM::Opcode::IMOV, 6, 9),
    makeInstruction(FlowVM::Opcode::SSUBSTR, 4, 3, 5),
    makeInstruction(FlowVM::Opcode::SPRINT, 4),
    makeInstruction(FlowVM::Opcode::NDUMPN, 0, 7),

    makeInstructionImm(FlowVM::Opcode::SCONST, 7, 4),   // r7 = sconst[4] /* "rl" */
    makeInstruction(FlowVM::Opcode::SCONTAINS, 8, 3, 4),
    makeInstruction(FlowVM::Opcode::NDUMPN, 7, 2),

    makeInstructionImm(FlowVM::Opcode::EXIT, 1),
};

/*
 * IMOV r1, 1       ; argc
 * IMOV r2, 0       ; argv[0] result; (initialization not needed)
 * IMOV r0, 1       ; function ID of getcwd()I
 * CALL r0, r1, r2  ; fn#, argc, argv
 *
 *
 * ; r0=fid, r1=argc, r2=argv[0], r3=argv[1],
 *
 * MOV  r3, r2      ; argv[1]: result string from call above
 * IMOV r1, 2       ; argc
 * IMOV r0, 0       ; function ID of print(S)I
 * CALL r0, r1, r2  ; fn#, argc, argv
 *
 * EXIT 1
 */
static const std::vector<FlowVM::Instruction> code4 = {
    // /*r2*/ tmp = getcwd()
    makeInstructionImm(FlowVM::Opcode::IMOV, 1, 1), // argc
    makeInstructionImm(FlowVM::Opcode::IMOV, 2, 0), // argv[0]
    makeInstructionImm(FlowVM::Opcode::IMOV, 0, 1), // fid
    makeInstruction(FlowVM::Opcode::CALL, 0, 1, 2),

    // print(tmp)
    makeInstruction(FlowVM::Opcode::MOV, 3, 2),     // r3 = tmp
    makeInstructionImm(FlowVM::Opcode::IMOV, 1, 2), // argc
    makeInstructionImm(FlowVM::Opcode::IMOV, 0, 0), // fid
    makeInstruction(FlowVM::Opcode::CALL, 0, 1, 2),

    makeInstructionImm(FlowVM::Opcode::EXIT, 1),
};

/*
 * expr = true;
 * assert(expr, "Hello, World");
 * exit(0);
 *
 * IMOV r0, 0           ; handler ID
 * IMOV r1, 3           ; argc = 3
 * IMOV r3, 1           ; argv[1] = true ; expr result
 * SCONST r4, 1         ; argv[2] = sconst[1] // "Hello"
 * CALL r0, r1, r2
 * EXIT 0
 */
static const std::vector<FlowVM::Instruction> code5 = {
    makeInstructionImm(FlowVM::Opcode::IMOV, 0, 0),
    makeInstructionImm(FlowVM::Opcode::IMOV, 1, 3),
    makeInstructionImm(FlowVM::Opcode::IMOV, 3, 0), // expr result
    makeInstructionImm(FlowVM::Opcode::SCONST, 4, 1),
    makeInstruction(FlowVM::Opcode::HANDLER, 0, 1, 2),

    makeInstructionImm(FlowVM::Opcode::EXIT, 0),
};

class FlowTest : public FlowVM::Runtime {
public:
    FlowTest()
    {
        registerHandler("assert")
            .signature(FlowVM::Type::Boolean, FlowVM::Type::String)
            .bind(&FlowTest::_assert);

        registerFunction("getcwd", FlowVM::Type::String)
            .bind(&FlowTest::_getcwd);

        registerFunction("print", FlowVM::Type::Number)
            .signature(FlowVM::Type::String)
            .bind(&FlowTest::_print);
    }

    virtual bool import(const std::string& name, const std::string& path)
    {
        printf("FlowTest: about to import plugin '%s' from path '%s' (no-op)\n",
                name.c_str(), path.c_str());

        return true;
    }

    // signature:
    //     "assert(BS)B"
    //      bool assert(bool exprResult, string exprSourceCode);
    void _assert(int argc, FlowVM::Value* argv, FlowVM::Runner* cx)
    {
        printf("assertion: %-6s; %s\n",
            argv[1] ? "true" : "false",
            ((FlowVM::String*)argv[2])->c_str());
        argv[0] = argv[1];
    }

    void _print(int argc, FlowVM::Value* argv, FlowVM::Runner* cx)
    {
        printf("%s\n", ((FlowVM::String*)argv[1])->c_str());
    }

    void _getcwd(int argc, FlowVM::Value* argv, FlowVM::Runner* cx)
    {
        char cwd[PATH_MAX];
        getcwd(cwd, sizeof(cwd));
        argv[0] = (FlowVM::Value) cx->createString(cwd);
    }
};

int main()
{
    FlowVM::Program program(
        {123456789, 56789},                 // integer constants
        {"", "Hello", "World", " ", "rl"},  // string constants
        {"^H.ll. W.rld$"},                  // regex constants
        {{"fnord", ""},                     // external modules
         {"foo", "/usr/libexec"}},
        {"assert(BS)B"},                    // native handler signatures
        {"print(S)I", "getcwd()S"}          // native function signatures
    );

    program.createHandler("test1", code1); // simple
    program.createHandler("test2", code2); // number math iteration test
    program.createHandler("test3", code3); // string test
    program.createHandler("test4", code4); // function call test
    program.createHandler("test5", code5); // handler call test

    FlowTest runtime;
    program.link(&runtime);

    if (FlowVM::Handler* handler = program.findHandler("test5")) {
        printf("Disassembling %s ...\n", handler->signature().c_str());
        handler->disassemble();

        printf("\nRunning %s ...\n", handler->signature().c_str());
        std::unique_ptr<FlowVM::Runner> flow = handler->createRunner();
        flow->run();
    }

    return 0;
}
