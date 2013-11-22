## Flow Virtual Machine

### Instruction Stream

The program is stored as an array (stream) of fixed-length 32-bit instructions.
Instructions are represented as follows:

    32  24  16  8    0
    +---+---+---+----+
    |           | OP |      Instruction with no register operands.
    +---+---+---+----+

    32  24  16  8    0
    +---+---+---+----+
    |       | A | OP |      Instruction with 1 register operands.
    +---+---+---+----+

    32  24  16  8    0
    +---+---+---+----+
    |   | B | A | OP |      Instruction with 2 register operands.
    +---+---+---+----+

    32  24  16  8    0
    +---+---+---+----+
    | C | B | A | OP |      Instruction with 3 register operands.
    +---+---+---+----+

    32      16  8    0
    +-------+---+----+
    | D     | A | OP |      Instruction with 1 register operand and a 16-bit immediate literal.
    +---+---+---+----+

### Constants

Constants are all stored in a constant table.

- integer constants: 64 bite signed
- string constants: C (null-terminated) string with a pre-computed string length stored also

### Opcodes

#### Instruction Prefixes

 - *V* - variable
 - *N* - integer constant
 - *I* - immediate 16-bit integer literal
 - *S* - string constant

#### Instruction Operand Types

 - *vdest* - variable slot, used as destination
 - *var* - variable slot
 - *imm* - immediate 16-bit integer literal
 - *num* - number constant slot
 - *str* - string constant slot
 - *pc* - jump offset into program bytecode

#### Constant Ops

    Opcode  Mnemonic  A       D           Description
    --------------------------------------------------------------------------------------------
    0x??    NMOV      vdest   num         set integer A to integer constant D
    0x??    SMOV      vdest   str         set string A to string constant D
    0x??    NCLEAR    vdest   -           set integer A to 0.
    0x??    SCLEAR    vdest   -           set string A to empty string.

    Opcode  Mnemonic  A       B           Description
    --------------------------------------------------------------------------------------------
    0x??    IMOV      vdest   imm         set integer A to immediate 16-bit integer literal D

#### Unary Ops

    Opcode  Mnemonic  A       D           Description
    --------------------------------------------------------------------------------------------
    0x??    VMOV      vdest   var         A = D
    0x??    VNOT      vdest   var         A = !D
    0x??    VNEG      vdest   var         A = -D
    0x??    SLEN      vdest   var         A = strlen(D)

#### Binary Numerical Ops

    Opcode  Mnemonic  A       B     C     Description
    --------------------------------------------------------------------------------------------
    0x??    NADD      vdest   var   var   A = B + C
    0x??    NSUB      vdest   var   var   A = B - C
    0x??    NMUL      vdest   var   var   A = B * C
    0x??    NDIV      vdest   var   var   A = B / C
    0x??    NREM      vdest   var   var   A = B % C
    0x??    NSHL      vdest   var   var   A = B << C
    0x??    NSHR      vdest   var   var   A = B >> C
    0x??    NPOW      vdest   var   var   A = B ** C
    0x??    NAND      vdest   var   var   A = B & C
    0x??    NOR       vdest   var   var   A = B | C
    0x??    NXOR      vdest   var   var   A = B ^ C
    0x??    NCMPEQ    vdest   var   var   A = B == C
    0x??    NCMPNE    vdest   var   var   A = B != C
    0x??    NCMPLE    vdest   var   var   A = B <= C
    0x??    NCMPGE    vdest   var   var   A = B >= C
    0x??    NCMPLT    vdest   var   var   A = B < C
    0x??    NCMPGT    vdest   var   var   A = B > C

#### Binary String Ops

    Opcode  Mnemonic  A       B     C     Description
    --------------------------------------------------------------------------------------------
    0x??    SCAT      vdest   var   var   A = B + C
    0x??    SCMPEQ    vdest   var   var   A = B == C
    0x??    SCMPNE    vdest   var   var   A = B != C
    0x??    SCMPLE    vdest   var   var   A = B <= C
    0x??    SCMPGE    vdest   var   var   A = B >= C
    0x??    SCMPLT    vdest   var   var   A = B < C
    0x??    SCMPGT    vdest   var   var   A = B > C
    0x??    SCMPBEG   vdest   var   var   A = B =^ C
    0x??    SCMPEND   vdest   var   var   A = B =$ C
    0x??    SCMPSET   vdest   var   var   A = B in C
    0x??    RMATCH    vdest   var   var   A = B =~ C

### Control Ops

    Opcode  Mnemonic  A       B     C     Description
    --------------------------------------------------------------------------------------------
    0x??    EXIT      imm     -     -     End program with given return boolean status code
    0x??    HANDLER   -       -     -     Invoke native handler (can cause program to exit)
    0x??    FUNCTIONI vdest   -     -     Invoke native func, returns an integer, takes no args
    0x??    FUNCTIONS vdest   -     -     Invoke native func; returns a string, takes no args

