#include <flow/Instruction.h>
#include <initializer_list>
#include <vector>
#include <utility>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <new>

using namespace flow;

class FlowContext;
class FlowProgram;

class FlowProgram // {{{
{
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
     * u64                  debug source-lines-table start
     * u64                  debug source-lines-table element count
     *
     * u32[]                code segment
     * u64[]                integer const-table segment
     * u64[]                string const-table segment
     * {u32, u8[]}[]        strings
     * {u32, u32, u32}[]    debug source lines segment
     */ // }}}
public:
    FlowProgram();
    FlowProgram(
        const std::vector<Instruction>& instructions,
        const std::vector<uint64_t>& constNumbers,
        const std::vector<std::pair<char*, size_t>>& constStrings,
        size_t numRegisters);
    ~FlowProgram();

    const std::vector<Instruction>& instructions() const;
    const std::vector<uint64_t>& numbers() const;
    const std::vector<std::pair<char*, size_t>>& strings() const;
    size_t registerCount() const;

    FlowContext* createContext();

private:
    std::vector<Instruction> instructions_;
    std::vector<uint64_t> numbers_;
    std::vector<std::pair<char*, size_t>> strings_;
    size_t registerCount_;
}; // }}}

typedef uint64_t FlowRegister;

struct FlowContext // {{{
{
    FlowProgram* program;
    void* userdata;
    FlowRegister data[];

    static FlowContext* create(FlowProgram* program) {
        FlowContext* cx = (FlowContext*) malloc(sizeof(FlowContext) + program->registerCount() * sizeof(uint64_t));
        new (cx) FlowContext(program);
        return cx;
    };

    static void operator delete (void* p) {
        free(p);
    }

    bool run();

private:
    FlowContext(FlowProgram* _program) : program(_program), userdata(nullptr) { }
}; // }}}

// {{{ FlowProgram impl
FlowProgram::FlowProgram() :
    instructions_(),
    numbers_(),
    strings_(),
    registerCount_(0)
{
}

FlowProgram::FlowProgram(
        const std::vector<Instruction>& instructions,
        const std::vector<uint64_t>& numbers,
        const std::vector<std::pair<char*, size_t>>& strings,
        size_t numRegisters) :
    instructions_(instructions),
    numbers_(numbers),
    strings_(strings),
    registerCount_(numRegisters)
{
}

FlowProgram::~FlowProgram()
{
}

inline const std::vector<Instruction>& FlowProgram::instructions() const
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

size_t FlowProgram::registerCount() const
{
    return registerCount_;
}

FlowContext* FlowProgram::createContext()
{
    return FlowContext::create(this);
}
// }}}

void disassemble(Instruction pc, size_t ip, const char* comment = nullptr)
{
    Opcode opc = opcode(pc);
    Operand A = operandA(pc);
    Operand B = operandB(pc);
    Operand C = operandC(pc);
    ImmOperand D = operandD(pc);
    const char* mnemo = mnemonic(opc);
    size_t n = 0;
    int rv = 0;

    rv = printf(" %3zu: %-10s", ip, mnemo);
    if (rv > 0) {
        n += rv;
    }

    switch (operandSignature(opc)) {
        case Signature::None: break;
        case Signature::R:    rv = printf(" r%d", A); break;
        case Signature::RR:   rv = printf(" r%d, r%d", A, B); break;
        case Signature::RRR:  rv = printf(" r%d, r%d, r%d", A, B, C); break;
        case Signature::RI:   rv = printf(" r%d, %d", A, D); break;
        case Signature::I:    rv = printf(" %d", D); break;
    }

    if (rv > 0) {
        n += rv;
    }

    if (comment && *comment) {
        for (; n < 30; ++n) {
            printf(" ");
        }
        printf("; %s\n", comment);
    } else {
        printf("\n");
    }
}

void disassemble(const Instruction* program, size_t n)
{
    size_t i = 0;
    for (const Instruction* pc = program; pc < program + n; ++pc) {
        disassemble(*pc, i++);
    }
}

bool FlowContext::run()
{
    // {{{ jump table
    static const void* ops[] = {
        // control
        [Opcode::EXIT]      = &&l_exit,
        [Opcode::JMP]       = &&l_jmp,
        [Opcode::CONDBR]    = &&l_condbr,

        // debug
        [Opcode::NDUMPN]    = &&l_ndumpn,

        // copy
        [Opcode::IMOV]      = &&l_imov,
        [Opcode::NMOV]      = &&l_nmov,
        [Opcode::NCONST]    = &&l_nconst,

        // binary: numerical
        [Opcode::NADD]      = &&l_nadd,
        [Opcode::NSUB]      = &&l_nsub,
        [Opcode::NMUL]      = &&l_nmul,
        [Opcode::NDIV]      = &&l_ndiv,
        [Opcode::NREM]      = &&l_nrem,
        [Opcode::NSHL]      = &&l_nshl,
        [Opcode::NSHR]      = &&l_nshr,
        [Opcode::NPOW]      = &&l_npow,
        [Opcode::NAND]      = &&l_nand,
        [Opcode::NOR]       = &&l_nor,
        [Opcode::NXOR]      = &&l_nxor,
        [Opcode::NCMPEQ]    = &&l_ncmpeq,
        [Opcode::NCMPNE]    = &&l_ncmpne,
        [Opcode::NCMPLE]    = &&l_ncmple,
        [Opcode::NCMPGE]    = &&l_ncmpge,
        [Opcode::NCMPLT]    = &&l_ncmplt,
        [Opcode::NCMPGT]    = &&l_ncmpgt,
    };
    // }}}

    register const Instruction* pc = program->instructions().data();
    size_t icount = 0;

    #define INSTR do { icount++; disassemble(*pc, pc - program->instructions().data()); } while (0)
    #define INSTR2(op, x, y) \
        do { \
            char buf[80]; \
            snprintf(buf, sizeof(buf), "%li %s %li", \
                    data[x], op, data[y]); \
            icount++; \
            disassemble(*pc, pc - program->instructions().data(), buf); \
        } while (0)
    #define OP opcode(*pc)
    #define A  operandA(*pc)
    #define B  operandB(*pc)
    #define C  operandC(*pc)
    #define D  operandD(*pc)
    #define next goto *ops[opcode(*++pc)]

    goto *ops[OP];

    // {{{ control
l_exit:
    INSTR;
    printf("exiting program. ran %zu instructions\n", icount);
    return D != 0;

l_jmp:
    INSTR;
    pc = program->instructions().data() + D;
    goto *ops[OP];

l_condbr:
    INSTR;
    if (data[A] != 0) {
        pc = program->instructions().data() + D;
        goto *ops[OP];
    } else {
        next;
    }
    // }}}
    // {{{ copy
l_imov:
    INSTR;
    data[A] = D;
    next;

l_nmov:
    INSTR;
    data[A] = data[B];
    next;

l_nconst:
    INSTR;
    data[A] = program->numbers()[D];
    next;
    // }}}
    // {{{ debug
l_ndumpn:
    INSTR;
    printf("regdump: ");
    for (int i = 0; i < B; ++i) {
        if (i) printf(", ");
        printf("r%d = %li", A + i, (int64_t)data[A + i]);
    }
    if (B) printf("\n");
    next;
    // }}}
    // {{{ binary numerical
l_nadd:
    INSTR2("+", B, C);
    data[A] = data[B] + data[C];
    next;

l_nsub:
    INSTR2("-", B, C);
    data[A] = data[B] - data[C];
    next;

l_nmul:
    INSTR2("*", B, C);
    data[A] = data[B] * data[C];
    next;

l_ndiv:
    INSTR2("/", B, C);
    data[A] = data[B] / data[C];
    next;

l_nrem:
    INSTR2("%", B, C);
    data[A] = data[B] % data[C];
    next;

l_nshl:
    INSTR2("<<", B, C);
    data[A] = data[B] << data[C];
    next;

l_nshr:
    INSTR2(">>", B, C);
    data[A] = data[B] >> data[C];
    next;

l_npow:
    INSTR2("**", B, C);
    data[A] = powl(data[B], data[C]);
    next;

l_nand:
    INSTR2("&", B, C);
    data[A] = data[B] & data[C];
    next;

l_nor:
    INSTR2("|", B, C);
    data[A] = data[B] | data[C];
    next;

l_nxor:
    INSTR2("^", B, C);
    data[A] = data[B] ^ data[C];
    next;

l_ncmpeq:
    INSTR2("==", B, C);
    data[A] = data[B] == data[C];
    next;

l_ncmpne:
    INSTR2("!=", B, C);
    data[A] = data[B] != data[C];
    next;

l_ncmple:
    INSTR2("<=", B, C);
    data[A] = data[B] <= data[C];
    next;

l_ncmpge:
    INSTR2(">=", B, C);
    data[A] = data[B] >= data[C];
    next;

l_ncmplt:
    INSTR2("<", B, C);
    data[A] = data[B] < data[C];
    next;

l_ncmpgt:
    INSTR2(">", B, C);
    data[A] = data[B] > data[C];
    next;
    // }}}
}

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
static const std::vector<Instruction> code2 = {
    // prolog
    makeInstructionImm(Opcode::IMOV, 0, 4),     // r0 = 4
    makeInstructionImm(Opcode::IMOV, 1, 0),     // r1 = 0
    makeInstructionImm(Opcode::IMOV, 2, 0),     // r2 = 0
    makeInstructionImm(Opcode::IMOV, 4, 1),     // r4 = 1
    makeInstructionImm(Opcode::JMP, 7),         // IP = condition

    // loop body
    makeInstruction(Opcode::NADD, 1, 1, 4),     // r1 = r1 + 1
    makeInstruction(Opcode::NADD, 2, 2, 1),     // r2 = r2 + r1

    // condition
    makeInstruction(Opcode::NCMPLT, 3, 1, 0),   // r3 = r1 < r0 ; 4
    makeInstructionImm(Opcode::CONDBR, 3, 5),   // if isTrue(r3) then IP = loopBody

    // epilog
    makeInstruction(Opcode::NDUMPN, 0, 5),

    makeInstruction(Opcode::NCONST, 0, 0),      // r0 = nconst[0]
    makeInstruction(Opcode::NCONST, 1, 1),      // r1 = nconst[1]
    makeInstruction(Opcode::NSUB, 2, 0, 1),     // r2 = r0 - r1
    makeInstruction(Opcode::NDUMPN, 0, 3),

    makeInstructionImm(Opcode::EXIT, 1),
};

int main()
{
    FlowProgram p(code2, {123456789, 56789}, {}, 32);

    printf("Disassembling program (%zi instructions)\n\n", p.instructions().size());
    disassemble(p.instructions().data(), p.instructions().size());
    printf("\nRunning program\n");

    FlowContext* cx = p.createContext();
    bool rv = cx->run();
    delete cx;

    return 0;
}
