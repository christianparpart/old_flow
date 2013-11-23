#include <flow/Instruction.h>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <new>

using namespace flow;

void disassemble(Instruction pc, size_t ip)
{
    Opcode opc = opcode(pc);
    Operand A = operandA(pc);
    Operand B = operandB(pc);
    Operand C = operandC(pc);
    ImmOperand D = operandD(pc);
    const char* mnemo = mnemonic(opc);

    switch (operandSignature(opc)) {
        case Signature::None: printf(" %3zu: %-10s\n", ip, mnemo); break;
        case Signature::R:    printf(" %3zu: %-10s r%d\n", ip, mnemo, A); break;
        case Signature::RR:   printf(" %3zu: %-10s r%d, r%d\n", ip, mnemo, A, B); break;
        case Signature::RRR:  printf(" %3zu: %-10s r%d, r%d, r%d\n", ip, mnemo, A, B, C); break;
        case Signature::RI:   printf(" %3zu: %-10s r%d, %d\n", ip, mnemo, A, D); break;
        case Signature::I:    printf(" %3zu: %-10s %d\n", ip, mnemo, D); break;
    }
}

void disassemble(const Instruction* program, size_t n)
{
    size_t i = 0;
    for (const Instruction* pc = program; pc < program + n; ++pc) {
        disassemble(*pc, i++);
    }
}

struct VMContext {
    void* userdata;
    size_t dataSize;
    uint64_t data[];

    static VMContext* create(size_t slots) {
        VMContext* p = (VMContext*) malloc(sizeof(VMContext) + slots * sizeof(uint64_t));
        new (p) VMContext(slots);
        return p;
    };

    static void operator delete (void* p) {
        free(p);
    }

    bool run(const Instruction* program);

private:
    VMContext(size_t slots) : userdata(nullptr), dataSize(slots) { }
};

bool VMContext::run(const Instruction* program)
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

    register const Instruction* pc = program;
    size_t icount = 0;

    #define INSTR do { icount++; disassemble(*pc, pc - program); } while (0)
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
    pc = program + D;
    goto *ops[OP];

l_condbr:
    INSTR;
    if (data[A] != 0) {
        pc = program + D;
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
    INSTR;
    data[A] = data[B] + data[C];
    next;

l_nsub:
    INSTR;
    data[A] = data[B] - data[C];
    next;

l_nmul:
    INSTR;
    data[A] = data[B] * data[C];
    next;

l_ndiv:
    INSTR;
    data[A] = data[B] / data[C];
    next;

l_nrem:
    INSTR;
    data[A] = data[B] % data[C];
    next;

l_nshl:
    INSTR;
    data[A] = data[B] << data[C];
    next;

l_nshr:
    INSTR;
    data[A] = data[B] >> data[C];
    next;

l_npow:
    INSTR;
    data[A] = powl(data[B], data[C]);
    next;

l_nand:
    INSTR;
    data[A] = data[B] & data[C];
    next;

l_nor:
    INSTR;
    data[A] = data[B] | data[C];
    next;

l_nxor:
    INSTR;
    data[A] = data[B] ^ data[C];
    next;

l_ncmpeq:
    INSTR;
    data[A] = data[B] == data[C];
    next;

l_ncmpne:
    INSTR;
    data[A] = data[B] != data[C];
    next;

l_ncmple:
    INSTR;
    data[A] = data[B] <= data[C];
    next;

l_ncmpge:
    INSTR;
    data[A] = data[B] >= data[C];
    next;

l_ncmplt:
    INSTR;
    data[A] = data[B] < data[C];
    next;

l_ncmpgt:
    INSTR;
    data[A] = data[B] > data[C];
    next;
    // }}}
}

static const Instruction program1[] = {
    makeInstructionImm(Opcode::IMOV, 1, 42),    // r1 = 42
    makeInstructionImm(Opcode::IMOV, 2, 7),     // r2 = 7
    makeInstruction(Opcode::NADD, 3, 1, 2),     // r3 = r1 + r2
    makeInstruction(Opcode::NDUMPN, 1, 3),      // regdump (r1 to r3)
    makeInstruction(Opcode::NMOV, 2, 1),        // r2 = r1
    makeInstruction(Opcode::NMOV, 3, 1),        // r3 = r1
    makeInstruction(Opcode::NDUMPN, 1, 3),      // regdump (r1 to r3)
    makeInstructionImm(Opcode::EXIT, 1),        // EXIT true
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
static const Instruction program2[] = {
    // prolog
    makeInstruction(Opcode::IMOV, 0, 4),        // r0 = 4
    makeInstruction(Opcode::IMOV, 1, 0),        // r1 = 0
    makeInstruction(Opcode::IMOV, 2, 0),        // r2 = 0
    makeInstruction(Opcode::IMOV, 4, 1),        // r4 = 1
    makeInstructionImm(Opcode::JMP, 7),         // IP = condition

    // loop body
    makeInstruction(Opcode::NADD, 1, 1, 4),     // r1 = r1 + 1
    makeInstruction(Opcode::NADD, 2, 2, 1),     // r2 = r2 + r1

    // condition
    makeInstruction(Opcode::NCMPLT, 3, 1, 0),   // r3 = r1 < r0 ; 4
    makeInstructionImm(Opcode::CONDBR, 3, 5),   // if isTrue(r3) then IP = loopBody

    // epilog
    makeInstruction(Opcode::NDUMPN, 0, 5),
    makeInstructionImm(Opcode::EXIT, 1),
};

int main()
{

    printf("Disassembling program (%zi instructions)\n\n", sizeof(program2) / sizeof(*program2));
    disassemble(program2, sizeof(program2) / sizeof(*program2));

    VMContext* cx = VMContext::create(32);
    printf("\nRunning program\n");
    bool rv = cx->run(program2);
    printf("%s\n", rv ? "\nSuccess" : "\nFailed");
    delete cx;

    return 0;
}
