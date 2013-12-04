#pragma once

#include <sys/param.h> // size_t, odd that it's not part of <stdint.h>.
#include <stdint.h>
#include <vector>

namespace FlowVM {

enum Opcode {
    // control
    EXIT = 0,       // exit program
    JMP,            // unconditional jump
    CONDBR,         // conditional jump

    // debugging
    NTICKS,         // instruction performance counter
    NDUMPN,         // dump registers range [A .. (D - A)]

    // copy
    MOV,            // A = B

    // numerical
    IMOV,           // A = D/imm
    NCONST,         // A = numberConstants[D]
    NNEG,           // A = -A
    NADD,           // A = B + C
    NSUB,           // A = B - C
    NMUL,           // A = B * C
    NDIV,           // A = B / C
    NREM,           // A = B % C
    NSHL,           // A = B << C
    NSHR,           // A = B >> C
    NPOW,           // A = B ** C
    NAND,           // A = B & C
    NOR,            // A = B | C
    NXOR,           // A = B ^ C
    NCMPEQ,         // A = B == C
    NCMPNE,         // A = B != C
    NCMPLE,         // A = B <= C
    NCMPGE,         // A = B >= C
    NCMPLT,         // A = B < C
    NCMPGT,         // A = B > C

    // string
    SCONST,         // A = stringConstants[D]
    SADD,           // A = B + C
    SADDMULTI,      // A = concat(B /*rbase*/, C /*count*/)
    SSUBSTR,        // A = substr(B, C /*offset*/, C+1 /*count*/)
    SCMPEQ,         // A = B == C
    SCMPNE,         // A = B != C
    SCMPLE,         // A = B <= C
    SCMPGE,         // A = B >= C
    SCMPLT,         // A = B < C
    SCMPGT,         // A = B > C
    SCMPBEG,        // A = B =^ C           /* B begins with C */
    SCMPEND,        // A = B =$ C           /* B ends with C */
    SCONTAINS,      // A = B in C           /* B is contained in C */
    SLEN,           // A = strlen(B)
    SPRINT,         // puts(A)              /* prints string A to stdout */

    // regex
    SREGMATCH,      // A = B =~ C           /* regex match against regexPool[C] */
    SREGGROUP,      // A = regex.match(B)   /* regex match result */

    // conversion
    I2S,            // A = itoa(B)
    S2I,            // A = atoi(B)
    SURLENC,        // A = urlencode(B)
    SURLDEC,        // A = urldecode(B)

    // invokation
    // CALL A=id, B=argc, C=argv
    CALL,           // A = functions[B+0] (B+1 ... B+C)
    HANDLER,        // if (handlers[B+0] (B+1 ... B+C)) EXIT 1
};

enum class InstructionSig {
    None = 0,   //               ()
    R,          // reg           (A)
    RR,         // reg, reg      (AB)
    RRR,        // reg, reg, reg (ABC)
    RI,         // reg, imm16    (AD)
    I,          // imm16         (D)
};

typedef uint32_t Instruction;
typedef uint8_t Operand;
typedef uint16_t ImmOperand;

// --------------------------------------------------------------------------
// encoder

constexpr Instruction makeInstruction(Opcode opc) { return (Instruction) opc; }
constexpr Instruction makeInstruction(Opcode opc, Operand op1) { return (opc | (op1 << 8)); }
constexpr Instruction makeInstruction(Opcode opc, Operand op1, Operand op2) { return (opc | (op1 << 8) | (op2 << 16)); }
constexpr Instruction makeInstruction(Opcode opc, Operand op1, Operand op2, Operand op3) { return (opc | (op1 << 8) | (op2 << 16) | (op3 << 24)); }
constexpr Instruction makeInstructionImm(Opcode opc, ImmOperand op2) { return (opc | (op2 << 16)); }
constexpr Instruction makeInstructionImm(Opcode opc, Operand op1, ImmOperand op2) { return (opc | (op1 << 8) | (op2 << 16)); }

// --------------------------------------------------------------------------
// decoder

void disassemble(Instruction pc, ImmOperand ip, const char* comment = nullptr);
void disassemble(const Instruction* program, size_t n);

constexpr Opcode opcode(Instruction instr) { return static_cast<Opcode>(instr & 0xFF); }
constexpr Operand operandA(Instruction instr) { return static_cast<Operand>((instr >> 8) & 0xFF); }
constexpr Operand operandB(Instruction instr) { return static_cast<Operand>((instr >> 16) & 0xFF); }
constexpr Operand operandC(Instruction instr) { return static_cast<Operand>((instr >> 24) & 0xFF); }
constexpr ImmOperand operandD(Instruction instr) { return static_cast<Operand>((instr >> 16) & 0xFFFF); }

inline InstructionSig operandSignature(Opcode opc);
inline const char* mnemonic(Opcode opc);

// --------------------------------------------------------------------------
// tools

size_t computeRegisterCount(const Instruction* code, size_t size);
size_t registerMax(Instruction instr);

// {{{ inlines
inline InstructionSig operandSignature(Opcode opc) {
    static const InstructionSig map[] = {
        // control
        [Opcode::EXIT]      = InstructionSig::I,
        [Opcode::JMP]       = InstructionSig::I,
        [Opcode::CONDBR]    = InstructionSig::RI,
        // debug
        [Opcode::NTICKS]    = InstructionSig::R,
        [Opcode::NDUMPN]    = InstructionSig::RI,
        // copy
        [Opcode::MOV]       = InstructionSig::RR,
        // numerical
        [Opcode::IMOV]      = InstructionSig::RI,
        [Opcode::NCONST]    = InstructionSig::RI,
        [Opcode::NNEG]      = InstructionSig::RR,
        [Opcode::NADD]      = InstructionSig::RRR,
        [Opcode::NSUB]      = InstructionSig::RRR,
        [Opcode::NMUL]      = InstructionSig::RRR,
        [Opcode::NDIV]      = InstructionSig::RRR,
        [Opcode::NREM]      = InstructionSig::RRR,
        [Opcode::NSHL]      = InstructionSig::RRR,
        [Opcode::NSHR]      = InstructionSig::RRR,
        [Opcode::NPOW]      = InstructionSig::RRR,
        [Opcode::NAND]      = InstructionSig::RRR,
        [Opcode::NOR]       = InstructionSig::RRR,
        [Opcode::NXOR]      = InstructionSig::RRR,
        [Opcode::NCMPEQ]    = InstructionSig::RRR,
        [Opcode::NCMPNE]    = InstructionSig::RRR,
        [Opcode::NCMPLE]    = InstructionSig::RRR,
        [Opcode::NCMPGE]    = InstructionSig::RRR,
        [Opcode::NCMPLT]    = InstructionSig::RRR,
        [Opcode::NCMPGT]    = InstructionSig::RRR,
        // string
        [Opcode::SCONST]    = InstructionSig::RI,
        [Opcode::SADD]      = InstructionSig::RRR,
        [Opcode::SSUBSTR]   = InstructionSig::RRR,
        [Opcode::SCMPEQ]    = InstructionSig::RRR,
        [Opcode::SCMPNE]    = InstructionSig::RRR,
        [Opcode::SCMPLE]    = InstructionSig::RRR,
        [Opcode::SCMPGE]    = InstructionSig::RRR,
        [Opcode::SCMPLT]    = InstructionSig::RRR,
        [Opcode::SCMPGT]    = InstructionSig::RRR,
        [Opcode::SCMPBEG]   = InstructionSig::RRR,
        [Opcode::SCMPEND]   = InstructionSig::RRR,
        [Opcode::SCONTAINS] = InstructionSig::RRR,
        [Opcode::SLEN]      = InstructionSig::R,
        [Opcode::SPRINT]    = InstructionSig::R,
        // regex
        [Opcode::SREGMATCH] = InstructionSig::RRR,
        [Opcode::SREGGROUP] = InstructionSig::R,
        // conversion
        [Opcode::I2S]       = InstructionSig::RR,
        [Opcode::S2I]       = InstructionSig::RR,
        [Opcode::SURLENC]   = InstructionSig::RR,
        [Opcode::SURLDEC]   = InstructionSig::RR,
        // invokation
        [Opcode::CALL]      = InstructionSig::RRR,
        [Opcode::HANDLER]   = InstructionSig::RRR,
    };
    return map[opc];
};

inline const char* mnemonic(Opcode opc) {
    static const char* map[] = {
        // control
        [Opcode::EXIT]   = "EXIT",
        [Opcode::JMP]    = "JMP",
        [Opcode::CONDBR] = "CONDBR",
        // copy
        [Opcode::MOV]    = "MOV",
        // debug
        [Opcode::NTICKS] = "NTICKS",
        [Opcode::NDUMPN] = "NDUMPN",
        // numerical
        [Opcode::IMOV]   = "IMOV",
        [Opcode::NCONST] = "NCONST",
        [Opcode::NNEG]   = "NNEG",
        [Opcode::NADD]   = "NADD",
        [Opcode::NSUB]   = "NSUB",
        [Opcode::NMUL]   = "NMUL",
        [Opcode::NDIV]   = "NDIV",
        [Opcode::NREM]   = "NREM",
        [Opcode::NSHL]   = "NSHL",
        [Opcode::NSHR]   = "NSHR",
        [Opcode::NPOW]   = "NPOW",
        [Opcode::NAND]   = "NADN",
        [Opcode::NOR]    = "NOR",
        [Opcode::NXOR]   = "NXOR",
        [Opcode::NCMPEQ] = "NCMPEQ",
        [Opcode::NCMPNE] = "NCMPNE",
        [Opcode::NCMPLE] = "NCMPLE",
        [Opcode::NCMPGE] = "NCMPGE",
        [Opcode::NCMPLT] = "NCMPLT",
        [Opcode::NCMPGT] = "NCMPGT",
        // string
        [Opcode::SCONST]    = "SCONST",
        [Opcode::SADD]      = "SADD",
        [Opcode::SSUBSTR]   = "SSUBSTR",
        [Opcode::SCMPEQ]    = "SCMPEQ",
        [Opcode::SCMPNE]    = "SCMPNE",
        [Opcode::SCMPLE]    = "SCMPLE",
        [Opcode::SCMPGE]    = "SCMPGE",
        [Opcode::SCMPLT]    = "SCMPLT",
        [Opcode::SCMPGT]    = "SCMPGT",
        [Opcode::SCMPBEG]   = "SCMPBEG",
        [Opcode::SCMPEND]   = "SCMPEND",
        [Opcode::SCONTAINS] = "SCONTAINS",
        [Opcode::SLEN]      = "SLEN",
        [Opcode::SPRINT]    = "SPRINT",
        // regex
        [Opcode::SREGMATCH] = "SREGMATCH",
        [Opcode::SREGGROUP] = "SREGGROUP",
        // conversion
        [Opcode::I2S]       = "I2S",
        [Opcode::S2I]       = "S2I",
        [Opcode::SURLENC]   = "SURLENC",
        [Opcode::SURLDEC]   = "SURLDEC",
        // invokation
        [Opcode::CALL]      = "CALL",
        [Opcode::HANDLER]   = "HANDLER",
    };
    return map[opc];
}
// }}}

} // namespace FlowVM
