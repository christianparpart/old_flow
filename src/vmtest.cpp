#include <flow/Instruction.h>
#include <cstdlib>
#include <cstdio>
#include <new>

using namespace flow;

void disassemble(const Instruction* program, size_t n)
{
    for (const Instruction* pc = program; pc < program + n; ++pc) {
        Opcode opc = opcode(*pc);
        Operand A = operandA(*pc);
        Operand B = operandB(*pc);
        Operand C = operandC(*pc);
        ImmOperand D = operandD(*pc);
        const char* mnemo = mnemonic(opc);

        switch (operandSignature(opc)) {
            case Signature::None: printf("  %-10s\n", mnemo); break;
            case Signature::R:    printf("  %-10s r%d\n", mnemo, A); break;
            case Signature::RR:   printf("  %-10s r%d, r%d\n", mnemo, A, B); break;
            case Signature::RRR:  printf("  %-10s r%d, r%d, r%d\n", mnemo, A, B, C); break;
            case Signature::RI:   printf("  %-10s r%d, %d\n", mnemo, A, D); break;
            case Signature::I:    printf("  %-10s %d\n", mnemo, D); break;
        }
    }
}

struct VMContext {
    void* userdata;
    uint64_t flags;
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
    VMContext(size_t slots) : userdata(nullptr), flags(0), dataSize(slots) { }
};

bool VMContext::run(const Instruction* program)
{
    static const void* ops[] = {
        [Opcode::IMOV] = &&l_imov,
        [Opcode::NMOV] = &&l_nmov,
        [Opcode::NADD] = &&l_nadd,
        [Opcode::NDUMPN] = &&l_ndumpn,
        [Opcode::EXIT] = &&l_exit,
    };

    register const Instruction* pc = program;

    #define OP opcode(*pc)
    #define A  operandA(*pc)
    #define B  operandB(*pc)
    #define C  operandC(*pc)
    #define D  operandD(*pc)
    #define next goto *ops[opcode(*++pc)]

    goto *ops[OP];

l_imov:
    data[A] = D;
    next;

l_nmov:
    data[A] = data[B];
    next;

l_nadd:
    data[A] = data[B] + data[C];
    next;

l_ndumpn:
    printf("regdump: ");
    for (int i = 0; i < B; ++i) {
        if (i) printf(", ");
        printf("r%d = %li", A + i, (int64_t)data[A + i]);
    }
    if (B) printf("\n");
    next;

l_exit:
    return D != 0;
}

static const Instruction program[] = {
    makeInstructionImm(Opcode::IMOV, 1, 42),    // r1 = 42
    makeInstructionImm(Opcode::IMOV, 2, 7),     // r2 = 7
    makeInstruction(Opcode::NADD, 3, 1, 2),     // r3 = r1 + r2
    makeInstruction(Opcode::NDUMPN, 1, 3),      // regdump (r1 to r3)
    makeInstruction(Opcode::NMOV, 2, 1),        // r2 = r1
    makeInstruction(Opcode::NMOV, 3, 1),        // r3 = r1
    makeInstruction(Opcode::NDUMPN, 1, 3),      // regdump (r1 to r3)
    makeInstructionImm(Opcode::EXIT, 1),        // EXIT true
};

int main()
{

    printf("Disassembling program (%zi instructions)\n\n", sizeof(program) / sizeof(*program));
    disassemble(program, sizeof(program) / sizeof(*program));

    VMContext* cx = VMContext::create(32);
    printf("\nRunning program\n");
    bool rv = cx->run(program);
    printf("%s\n", rv ? "\nSuccess" : "\nFailed");
    delete cx;

    return 0;
}
