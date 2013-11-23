#pragma once

#include <cstdint>

namespace flow {

enum Opcode {
    // control
    EXIT = 0,       // exit program
    JMP,            // unconditional jump
    CONDBR,         // conditional jump

    // debugging
    NDUMPN,         // dump registers range [A .. (D - A)]

    // copy
    IMOV,           // A = D/imm
    NMOV,           // A = B

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

enum class Signature {
    None = 0,
    R,      // reg           (A)
    RR,     // reg, reg      (AB)
    RRR,    // reg, reg, reg (ABC)
    RI,     // reg, imm16    (AD)
    I,      // imm16         (D)
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

constexpr Opcode opcode(Instruction instr) { return static_cast<Opcode>(instr & 0xFF); }
constexpr Operand operandA(Instruction instr) { return static_cast<Operand>((instr >> 8) & 0xFF); }
constexpr Operand operandB(Instruction instr) { return static_cast<Operand>((instr >> 16) & 0xFF); }
constexpr Operand operandC(Instruction instr) { return static_cast<Operand>((instr >> 24) & 0xFF); }
constexpr ImmOperand operandD(Instruction instr) { return static_cast<Operand>((instr >> 16) & 0xFFFF); }

inline Signature operandSignature(Opcode opc) {
    static const Signature map[] = {
        // control
        [Opcode::EXIT]      = Signature::I,
        [Opcode::JMP]       = Signature::I,
        [Opcode::CONDBR]    = Signature::RI,
        // debug
        [Opcode::NDUMPN]    = Signature::RI,
        // copy
        [Opcode::IMOV]      = Signature::RI,
        [Opcode::NMOV]      = Signature::RR,
        // binary numerical
        [Opcode::NADD]      = Signature::RRR,
        [Opcode::NSUB]      = Signature::RRR,
        [Opcode::NMUL]      = Signature::RRR,
        [Opcode::NDIV]      = Signature::RRR,
        [Opcode::NREM]      = Signature::RRR,
        [Opcode::NSHL]      = Signature::RRR,
        [Opcode::NSHR]      = Signature::RRR,
        [Opcode::NPOW]      = Signature::RRR,
        [Opcode::NAND]      = Signature::RRR,
        [Opcode::NOR]       = Signature::RRR,
        [Opcode::NXOR]      = Signature::RRR,
        [Opcode::NCMPEQ]    = Signature::RRR,
        [Opcode::NCMPNE]    = Signature::RRR,
        [Opcode::NCMPLE]    = Signature::RRR,
        [Opcode::NCMPGE]    = Signature::RRR,
        [Opcode::NCMPLT]    = Signature::RRR,
        [Opcode::NCMPGT]    = Signature::RRR,
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
        [Opcode::IMOV]   = "IMOV",
        [Opcode::NMOV]   = "NMOV",
        // debug
        [Opcode::NDUMPN] = "NDUMPN",
        // numeric
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
    };
    return map[opc];
}

} // namespace flow
