#include <flow/vm/Instruction.h>
#include <vector>
#include <utility>
#include <cstdlib>
#include <cstdio>
#include <climits>

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

/**
 * Retrieves the highest register as non-zero positive integer (1 to n), even though.
 */
size_t registerMax(Instruction instr)
{
    Operand result = 0;
    switch (operandSignature(opcode(instr))) {
        case InstructionSig::RRR:
            result = std::max(result, (Operand) (1 + operandC(instr)));
        case InstructionSig::RR:
            result = std::max(result, (Operand) (1 + operandB(instr)));
        case InstructionSig::R:
        case InstructionSig::RI:
            result = std::max(result, (Operand) (1 + operandA(instr)));
        case InstructionSig::I:
        case InstructionSig::None:
            return static_cast<size_t>(result);
    }
}

size_t computeRegisterCount(const Instruction* code, size_t size)
{
    auto* ip = code;
    auto* e = code + size;
    size_t count = 0;

    while (ip != e) {
        count = std::max(count, registerMax(*ip));
        ++ip;
    }

    return count;
}

} // namespace FlowVM
