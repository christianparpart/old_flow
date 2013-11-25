#pragma once

#include <cstdint>
#include <cstddef>

enum FlowOpcode {
    // control
    EXIT = 0,       // exit program
    JMP,            // unconditional jump
    CONDBR,         // conditional jump

    // debugging
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
};

enum class FlowInstructionSig {
    None = 0,   //               ()
    R,          // reg           (A)
    RR,         // reg, reg      (AB)
    RRR,        // reg, reg, reg (ABC)
    RI,         // reg, imm16    (AD)
    I,          // imm16         (D)
};

typedef uint32_t FlowInstruction;
typedef uint8_t FlowOperand;
typedef uint16_t FlowImmOperand;

// --------------------------------------------------------------------------
// encoder

constexpr FlowInstruction makeInstruction(FlowOpcode opc) { return (FlowInstruction) opc; }
constexpr FlowInstruction makeInstruction(FlowOpcode opc, FlowOperand op1) { return (opc | (op1 << 8)); }
constexpr FlowInstruction makeInstruction(FlowOpcode opc, FlowOperand op1, FlowOperand op2) { return (opc | (op1 << 8) | (op2 << 16)); }
constexpr FlowInstruction makeInstruction(FlowOpcode opc, FlowOperand op1, FlowOperand op2, FlowOperand op3) { return (opc | (op1 << 8) | (op2 << 16) | (op3 << 24)); }
constexpr FlowInstruction makeInstructionImm(FlowOpcode opc, FlowImmOperand op2) { return (opc | (op2 << 16)); }
constexpr FlowInstruction makeInstructionImm(FlowOpcode opc, FlowOperand op1, FlowImmOperand op2) { return (opc | (op1 << 8) | (op2 << 16)); }

// --------------------------------------------------------------------------
// decoder

void disassemble(FlowInstruction pc, size_t ip, const char* comment = nullptr);
void disassemble(const FlowInstruction* program, size_t n);

constexpr FlowOpcode opcode(FlowInstruction instr) { return static_cast<FlowOpcode>(instr & 0xFF); }
constexpr FlowOperand operandA(FlowInstruction instr) { return static_cast<FlowOperand>((instr >> 8) & 0xFF); }
constexpr FlowOperand operandB(FlowInstruction instr) { return static_cast<FlowOperand>((instr >> 16) & 0xFF); }
constexpr FlowOperand operandC(FlowInstruction instr) { return static_cast<FlowOperand>((instr >> 24) & 0xFF); }
constexpr FlowImmOperand operandD(FlowInstruction instr) { return static_cast<FlowOperand>((instr >> 16) & 0xFFFF); }

inline FlowInstructionSig operandSignature(FlowOpcode opc);
inline const char* mnemonic(FlowOpcode opc);

// {{{ inlines
inline FlowInstructionSig operandSignature(FlowOpcode opc) {
    static const FlowInstructionSig map[] = {
        // control
        [FlowOpcode::EXIT]      = FlowInstructionSig::I,
        [FlowOpcode::JMP]       = FlowInstructionSig::I,
        [FlowOpcode::CONDBR]    = FlowInstructionSig::RI,
        // debug
        [FlowOpcode::NDUMPN]    = FlowInstructionSig::RI,
        // copy
        [FlowOpcode::MOV]       = FlowInstructionSig::RR,
        // numerical
        [FlowOpcode::IMOV]      = FlowInstructionSig::RI,
        [FlowOpcode::NCONST]    = FlowInstructionSig::RI,
        [FlowOpcode::NNEG]      = FlowInstructionSig::RR,
        [FlowOpcode::NADD]      = FlowInstructionSig::RRR,
        [FlowOpcode::NSUB]      = FlowInstructionSig::RRR,
        [FlowOpcode::NMUL]      = FlowInstructionSig::RRR,
        [FlowOpcode::NDIV]      = FlowInstructionSig::RRR,
        [FlowOpcode::NREM]      = FlowInstructionSig::RRR,
        [FlowOpcode::NSHL]      = FlowInstructionSig::RRR,
        [FlowOpcode::NSHR]      = FlowInstructionSig::RRR,
        [FlowOpcode::NPOW]      = FlowInstructionSig::RRR,
        [FlowOpcode::NAND]      = FlowInstructionSig::RRR,
        [FlowOpcode::NOR]       = FlowInstructionSig::RRR,
        [FlowOpcode::NXOR]      = FlowInstructionSig::RRR,
        [FlowOpcode::NCMPEQ]    = FlowInstructionSig::RRR,
        [FlowOpcode::NCMPNE]    = FlowInstructionSig::RRR,
        [FlowOpcode::NCMPLE]    = FlowInstructionSig::RRR,
        [FlowOpcode::NCMPGE]    = FlowInstructionSig::RRR,
        [FlowOpcode::NCMPLT]    = FlowInstructionSig::RRR,
        [FlowOpcode::NCMPGT]    = FlowInstructionSig::RRR,
        // string
        [FlowOpcode::SCONST]    = FlowInstructionSig::RI,
        [FlowOpcode::SADD]      = FlowInstructionSig::RRR,
        [FlowOpcode::SSUBSTR]   = FlowInstructionSig::RRR,
        [FlowOpcode::SCMPEQ]    = FlowInstructionSig::RRR,
        [FlowOpcode::SCMPNE]    = FlowInstructionSig::RRR,
        [FlowOpcode::SCMPLE]    = FlowInstructionSig::RRR,
        [FlowOpcode::SCMPGE]    = FlowInstructionSig::RRR,
        [FlowOpcode::SCMPLT]    = FlowInstructionSig::RRR,
        [FlowOpcode::SCMPGT]    = FlowInstructionSig::RRR,
        [FlowOpcode::SCMPBEG]   = FlowInstructionSig::RRR,
        [FlowOpcode::SCMPEND]   = FlowInstructionSig::RRR,
        [FlowOpcode::SCONTAINS] = FlowInstructionSig::RRR,
        [FlowOpcode::SLEN]      = FlowInstructionSig::R,
        [FlowOpcode::SPRINT]    = FlowInstructionSig::R,
        // regex
        [FlowOpcode::SREGMATCH] = FlowInstructionSig::RRR,
        [FlowOpcode::SREGGROUP] = FlowInstructionSig::R,
        // conversion
        [FlowOpcode::I2S]       = FlowInstructionSig::RR,
        [FlowOpcode::S2I]       = FlowInstructionSig::RR,
        [FlowOpcode::SURLENC]   = FlowInstructionSig::RR,
        [FlowOpcode::SURLDEC]   = FlowInstructionSig::RR,
    };
    return map[opc];
};

inline const char* mnemonic(FlowOpcode opc) {
    static const char* map[] = {
        // control
        [FlowOpcode::EXIT]   = "EXIT",
        [FlowOpcode::JMP]    = "JMP",
        [FlowOpcode::CONDBR] = "CONDBR",
        // copy
        [FlowOpcode::MOV]    = "MOV",
        // debug
        [FlowOpcode::NDUMPN] = "NDUMPN",
        // numerical
        [FlowOpcode::IMOV]   = "IMOV",
        [FlowOpcode::NCONST] = "NCONST",
        [FlowOpcode::NNEG]   = "NNEG",
        [FlowOpcode::NADD]   = "NADD",
        [FlowOpcode::NSUB]   = "NSUB",
        [FlowOpcode::NMUL]   = "NMUL",
        [FlowOpcode::NDIV]   = "NDIV",
        [FlowOpcode::NREM]   = "NREM",
        [FlowOpcode::NSHL]   = "NSHL",
        [FlowOpcode::NSHR]   = "NSHR",
        [FlowOpcode::NPOW]   = "NPOW",
        [FlowOpcode::NAND]   = "NADN",
        [FlowOpcode::NOR]    = "NOR",
        [FlowOpcode::NXOR]   = "NXOR",
        [FlowOpcode::NCMPEQ] = "NCMPEQ",
        [FlowOpcode::NCMPNE] = "NCMPNE",
        [FlowOpcode::NCMPLE] = "NCMPLE",
        [FlowOpcode::NCMPGE] = "NCMPGE",
        [FlowOpcode::NCMPLT] = "NCMPLT",
        [FlowOpcode::NCMPGT] = "NCMPGT",
        // string
        [FlowOpcode::SCONST]    = "SCONST",
        [FlowOpcode::SADD]      = "SADD",
        [FlowOpcode::SSUBSTR]   = "SSUBSTR",
        [FlowOpcode::SCMPEQ]    = "SCMPEQ",
        [FlowOpcode::SCMPNE]    = "SCMPNE",
        [FlowOpcode::SCMPLE]    = "SCMPLE",
        [FlowOpcode::SCMPGE]    = "SCMPGE",
        [FlowOpcode::SCMPLT]    = "SCMPLT",
        [FlowOpcode::SCMPGT]    = "SCMPGT",
        [FlowOpcode::SCMPBEG]   = "SCMPBEG",
        [FlowOpcode::SCMPEND]   = "SCMPEND",
        [FlowOpcode::SCONTAINS] = "SCONTAINS",
        [FlowOpcode::SLEN]      = "SLEN",
        [FlowOpcode::SPRINT]    = "SPRINT",
        // regex
        [FlowOpcode::SREGMATCH] = "SREGMATCH",
        [FlowOpcode::SREGGROUP] = "SREGGROUP",
        // conversion
        [FlowOpcode::I2S]       = "I2S",
        [FlowOpcode::S2I]       = "S2I",
        [FlowOpcode::SURLENC]   = "SURLENC",
        [FlowOpcode::SURLDEC]   = "SURLDEC",
    };
    return map[opc];
}
// }}}

