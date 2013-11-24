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

int main()
{
    FlowProgram p(code2, {123456789, 56789}, {}, 32);

    printf("Disassembling program (%zi instructions)\n\n", p.instructions().size());
    disassemble(p.instructions().data(), p.instructions().size());
    printf("\nRunning program\n");

    std::unique_ptr<FlowRunner> flow = p.createRunner();
    flow->run();

    return 0;
}
