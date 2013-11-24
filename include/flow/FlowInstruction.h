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
    IMOV,           // A = D/imm
    NMOV,           // A = B

    NCONST,         // A = numberConstants[D]
    SCONST,         // A = stringConstants[D]

    // binary: numerical
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
        [FlowOpcode::IMOV]      = FlowInstructionSig::RI,
        [FlowOpcode::NMOV]      = FlowInstructionSig::RR,
        [FlowOpcode::NCONST]    = FlowInstructionSig::RI,
        // binary numerical
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
        [FlowOpcode::IMOV]   = "IMOV",
        [FlowOpcode::NMOV]   = "NMOV",
        [FlowOpcode::NCONST] = "NCONST",
        // debug
        [FlowOpcode::NDUMPN] = "NDUMPN",
        // numeric
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
    };
    return map[opc];
}
// }}}

