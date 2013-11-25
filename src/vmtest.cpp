#include <flow/FlowProgram.h>
#include <flow/FlowRunner.h>
#include <flow/FlowInstruction.h>
#include <initializer_list>
#include <vector>
#include <utility>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <memory>
#include <new>

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
static const std::vector<FlowInstruction> code2 = {
    // prolog
    makeInstructionImm(FlowOpcode::IMOV, 0, 4),     // r0 = 4
    makeInstructionImm(FlowOpcode::IMOV, 1, 0),     // r1 = 0
    makeInstructionImm(FlowOpcode::IMOV, 2, 0),     // r2 = 0
    makeInstructionImm(FlowOpcode::IMOV, 4, 1),     // r4 = 1
    makeInstructionImm(FlowOpcode::JMP, 7),         // IP = condition

    // loop body
    makeInstruction(FlowOpcode::NADD, 1, 1, 4),     // r1 = r1 + 1
    makeInstruction(FlowOpcode::NADD, 2, 2, 1),     // r2 = r2 + r1

    // condition
    makeInstruction(FlowOpcode::NCMPLT, 3, 1, 0),   // r3 = r1 < r0 ; 4
    makeInstructionImm(FlowOpcode::CONDBR, 3, 5),   // if isTrue(r3) then IP = loopBody

    // epilog
    makeInstruction(FlowOpcode::NDUMPN, 0, 5),

    makeInstruction(FlowOpcode::NCONST, 0, 0),      // r0 = nconst[0]
    makeInstruction(FlowOpcode::NCONST, 1, 1),      // r1 = nconst[1]
    makeInstruction(FlowOpcode::NSUB, 2, 0, 1),     // r2 = r0 - r1
    makeInstruction(FlowOpcode::NDUMPN, 0, 3),

    makeInstructionImm(FlowOpcode::EXIT, 1),
};

/*
 * r0 = " "
 * r1 = "Hello"
 * r2 = "World"
 * r3 = r1 + r2
 * sdump r3
 *
 */
static const std::vector<FlowInstruction> code3 = {
    makeInstructionImm(FlowOpcode::SCONST, 0, 0),   // r1 = sconst[0]
    makeInstructionImm(FlowOpcode::SCONST, 1, 1),   // r1 = sconst[1]
    makeInstructionImm(FlowOpcode::SCONST, 2, 2),   // r2 = sconst[2]
    makeInstruction(FlowOpcode::SADD, 3, 1, 0),
    makeInstruction(FlowOpcode::SADD, 3, 3, 2),
    makeInstruction(FlowOpcode::SPRINT, 3),

    makeInstructionImm(FlowOpcode::IMOV, 5, 1),
    makeInstructionImm(FlowOpcode::IMOV, 6, 9),
    makeInstruction(FlowOpcode::SSUBSTR, 4, 3, 5),
    makeInstruction(FlowOpcode::SPRINT, 4),
    makeInstruction(FlowOpcode::NDUMPN, 0, 7),

    makeInstructionImm(FlowOpcode::SCONST, 7, 4),   // r7 = sconst[4] /* "rl" */
    makeInstruction(FlowOpcode::SCONTAINS, 8, 3, 4),
    makeInstruction(FlowOpcode::NDUMPN, 7, 2),

    makeInstructionImm(FlowOpcode::EXIT, 1),
};

int main()
{
    FlowProgram p(code2,
        {123456789, 56789},
        {" ", "Hello", "World", "!", "rl"},
        {}, 32);

    printf("Disassembling program (%zi instructions)\n\n", p.instructions().size());
    disassemble(p.instructions().data(), p.instructions().size());
    printf("\nRunning program\n");

    std::unique_ptr<FlowRunner> flow = p.createRunner();
    flow->run();

    return 0;
}
