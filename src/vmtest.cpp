#include <flow/Instruction.h>
#include <cstdio>

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

bool vmrun(const Instruction* program)
{
    static const void* ops[] = {
        [Opcode::IMOV] = &&l_imov,
        [Opcode::NADD] = &&l_nadd,
        [Opcode::NDUMPN] = &&l_ndumpn,
        [Opcode::EXIT] = &&l_exit,
    };

    uint64_t data[256];

    register const Instruction* pc = program;
    Opcode opc;
    Operand A, B, C;
    ImmOperand D;

    for (;;) {
        opc = opcode(*pc);
        A = operandA(*pc);
        B = operandB(*pc);
        C = operandC(*pc);
        D = operandD(*pc);

        pc++;

        goto *ops[opc];

    l_imov:
        data[A] = D;
        continue;

    l_nadd:
        data[A] = data[B] + data[C];
        continue;

    l_ndumpn:
        printf("regdump: ");
        for (int i = 0; i < B; ++i) {
            if (i) printf(", ");
            printf("r%d = %li", A + i, (int64_t)data[A + i]);
        }
        if (B) printf("\n");
        continue;

    l_exit:
        return D != 0;
    }
}

static const Instruction program[] = {
    makeInstructionImm(Opcode::IMOV, 1, 42),    // r1 = 42
    makeInstructionImm(Opcode::IMOV, 2, 7),     // r2 = 7
    makeInstruction(Opcode::NADD, 3, 1, 2),     // r3 = r1 + r2
    makeInstruction(Opcode::NDUMPN, 1, 3),      // regdump (r1 to r3)
    makeInstructionImm(Opcode::EXIT, 1),        // EXIT true
};

int main()
{
    printf("Disassembling program (%zi instructions)\n\n", sizeof(program) / sizeof(*program));
    disassemble(program, sizeof(program) / sizeof(*program));

    printf("\nRunning program\n");
    bool rv = vmrun(program);

    printf("%s\n", rv ? "\nSuccess" : "\nFailed");
    return 0;
}
