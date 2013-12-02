#include <flow/vm/Instruction.h>
#include <vector>
#include <utility>
#include <cstdlib>
#include <cstdio>

namespace FlowVM {

void disassemble(Instruction pc, ImmOperand ip, const char* comment)
{
    Opcode opc = opcode(pc);
    Operand A = operandA(pc);
    Operand B = operandB(pc);
    Operand C = operandC(pc);
    ImmOperand D = operandD(pc);
    const char* mnemo = mnemonic(opc);
    size_t n = 0;
    int rv = 0;

    rv = printf(" %3hu: %-10s", ip, mnemo);
    if (rv > 0) {
        n += rv;
    }

    switch (operandSignature(opc)) {
        case InstructionSig::None: break;
        case InstructionSig::R:    rv = printf(" r%d", A); break;
        case InstructionSig::RR:   rv = printf(" r%d, r%d", A, B); break;
        case InstructionSig::RRR:  rv = printf(" r%d, r%d, r%d", A, B, C); break;
        case InstructionSig::RI:   rv = printf(" r%d, %d", A, D); break;
        case InstructionSig::I:    rv = printf(" %d", D); break;
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

} // namespace FlowVM
