#include <flow/FlowInstruction.h>
#include <vector>
#include <utility>
#include <cstdlib>
#include <cstdio>

void disassemble(FlowInstruction pc, size_t ip, const char* comment)
{
    FlowOpcode opc = opcode(pc);
    FlowOperand A = operandA(pc);
    FlowOperand B = operandB(pc);
    FlowOperand C = operandC(pc);
    FlowImmOperand D = operandD(pc);
    const char* mnemo = mnemonic(opc);
    size_t n = 0;
    int rv = 0;

    rv = printf(" %3zu: %-10s", ip, mnemo);
    if (rv > 0) {
        n += rv;
    }

    switch (operandSignature(opc)) {
        case FlowInstructionSig::None: break;
        case FlowInstructionSig::R:    rv = printf(" r%d", A); break;
        case FlowInstructionSig::RR:   rv = printf(" r%d, r%d", A, B); break;
        case FlowInstructionSig::RRR:  rv = printf(" r%d, r%d, r%d", A, B, C); break;
        case FlowInstructionSig::RI:   rv = printf(" r%d, %d", A, D); break;
        case FlowInstructionSig::I:    rv = printf(" %d", D); break;
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

void disassemble(const FlowInstruction* program, size_t n)
{
    size_t i = 0;
    for (const FlowInstruction* pc = program; pc < program + n; ++pc) {
        disassemble(*pc, i++);
    }
}
