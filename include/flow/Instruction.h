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
    R,      // reg
    RR,     // reg, reg
    RRR,    // reg, reg, reg
    RI,     // reg, imm16
    I,      // imm16
};

enum class ArgsClass {
    None = 0,
    A,
    AB,
    ABC,
    AD,
};

typedef uint32_t Instruction;
typedef uint8_t Operand;
typedef uint16_t ImmOperand;

// encoder
constexpr Instruction makeInstruction(Opcode opc) { return (Instruction) opc; }
constexpr Instruction makeInstruction(Opcode opc, Operand op1) { return (opc | (op1 << 8)); }
constexpr Instruction makeInstruction(Opcode opc, Operand op1, Operand op2) { return (opc | (op1 << 8) | (op2 << 16)); }
constexpr Instruction makeInstruction(Opcode opc, Operand op1, Operand op2, Operand op3) { return (opc | (op1 << 8) | (op2 << 16) | (op3 << 24)); }
constexpr Instruction makeInstructionImm(Opcode opc, ImmOperand op2) { return (opc | (op2 << 16)); }
constexpr Instruction makeInstructionImm(Opcode opc, Operand op1, ImmOperand op2) { return (opc | (op1 << 8) | (op2 << 16)); }

// decoder
constexpr Opcode opcode(Instruction instr) { return static_cast<Opcode>(instr & 0xFF); }
constexpr Operand operandA(Instruction instr) { return static_cast<Operand>((instr >> 8) & 0xFF); }
constexpr Operand operandB(Instruction instr) { return static_cast<Operand>((instr >> 16) & 0xFF); }
constexpr Operand operandC(Instruction instr) { return static_cast<Operand>((instr >> 24) & 0xFF); }
constexpr ImmOperand operandD(Instruction instr) { return static_cast<Operand>((instr >> 16) & 0xFFFF); }

inline Signature operandSignature(Opcode opc) {
    static const Signature map[] = {
        [Opcode::IMOV]   = Signature::RI,
        [Opcode::NADD]   = Signature::RRR,
        [Opcode::NDUMPN] = Signature::RI,
        [Opcode::EXIT]   = Signature::I,
    };
    return map[opc];
};

inline ArgsClass operandArgsClass(Opcode opc) {
    static const ArgsClass map[] = {
        [Opcode::IMOV]   = ArgsClass::AD,
        [Opcode::NADD]   = ArgsClass::ABC,
        [Opcode::NDUMPN] = ArgsClass::AB,
        [Opcode::EXIT]   = ArgsClass::A,
    };
    return map[opc];
};

inline const char* mnemonic(Opcode opc) {
    static const char* map[] = {
        [Opcode::IMOV]   = "IMOV",
        [Opcode::NADD]   = "NADD",
        [Opcode::NDUMPN] = "NDUMPN",
        [Opcode::EXIT]   = "EXIT",
    };
    return map[opc];
}

} // namespace flow
