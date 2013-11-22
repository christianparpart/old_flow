## Flow Virtual Machine

### TODO

- use 64-bit instruction width instead of 32-bit
- use 16-bit width register identification instead of 8-bit inside the opcode, effectively raising the register limit.
- reconsider native call mechanism to (maybe) use dedicated opcodes for core native functions/handlers.

### Instruction Stream

The program is stored as an array (stream) of fixed-length 32-bit instructions.

The opcode always takes the lowest 8-bit of the instruction, also determining the
interpretation of the higher 24 bits to one of the above variations.

A register is always represented as an 8-bit index to the register array, effectively limiting
the total number of registers to 256 while the first register contains always the flow control context.

Registers does not necessarily require them to be located in a CPU hardware
but can also be represented as software array.

In the following tables, the values have the following meaning:

- *OP*: opcode
- *A*: first operand
- *B*: second operand
- *D*: second operand (alternative)
- *C*: third operand (illegal if *D* is used)

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

    32  24  16  8    0
    +---+---+---+----+
    |   D   | A | OP |      Instruction with 1 register operand and a 16-bit immediate literal.
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

 - *var* - variable slot
 - *vres* - variable slot, used as destination
 - *vbase* - variable slot, the first of a consecutive list of variables
 - *num* - number constant slot
 - *str* - string constant slot
 - *imm* - immediate literal
 - *pc* - jump offset into program bytecode

#### Constant Ops

    Opcode  Mnemonic  A       D             Description
    --------------------------------------------------------------------------------------------
    0x??    NMOV      vres    num           set integer A to integer constant D
    0x??    SMOV      vres    str           set string A to string constant D
    0x??    NCLEAR    vres    -             set integer A to 0.
    0x??    SCLEAR    vres    -             set string A to empty string.

    Opcode  Mnemonic  A       B             Description
    --------------------------------------------------------------------------------------------
    0x??    IMOV      vres    imm           set integer A to immediate 16-bit integer literal D

#### Cast Ops

    0x??    S2I       vres    var           A = atoi(D)
    0x??    I2S       vres    var           A = itoa(D)
    0x??    I2B       vres    var           A = (D != 0) ? 1 : 0

#### Unary Ops

    Opcode  Mnemonic  A       D             Description
    --------------------------------------------------------------------------------------------
    0x??    VMOV      vres    var           A = D
    0x??    VNEG      vres    var           A = -D
    0x??    VNOT      vres    var           A = ~B
    0x??    SLEN      vres    var           A = strlen(D)

#### Binary Numerical Ops

    Opcode  Mnemonic  A       B     C       Description
    --------------------------------------------------------------------------------------------
    0x??    NADD      vres    var   var     A = B + C
    0x??    NSUB      vres    var   var     A = B - C
    0x??    NMUL      vres    var   var     A = B * C
    0x??    NDIV      vres    var   var     A = B / C
    0x??    NREM      vres    var   var     A = B % C
    0x??    NSHL      vres    var   var     A = B << C
    0x??    NSHR      vres    var   var     A = B >> C
    0x??    NPOW      vres    var   var     A = B ** C
    0x??    NAND      vres    var   var     A = B & C
    0x??    NOR       vres    var   var     A = B | C
    0x??    NXOR      vres    var   var     A = B ^ C
    0x??    NCMPEQ    vres    var   var     A = B == C
    0x??    NCMPNE    vres    var   var     A = B != C
    0x??    NCMPLE    vres    var   var     A = B <= C
    0x??    NCMPGE    vres    var   var     A = B >= C
    0x??    NCMPLT    vres    var   var     A = B < C
    0x??    NCMPGT    vres    var   var     A = B > C

#### Binary String Ops

    Opcode  Mnemonic  A       B     C       Description
    --------------------------------------------------------------------------------------------
    0x??    SCAT      vres    var   var     A = B + C
    0x??    SSUBSTR   vres    var           A = substr(B, C /* offset */, C + 1 /* count */)
    0x??    SCMPEQ    vres    var   var     A = B == C
    0x??    SCMPNE    vres    var   var     A = B != C
    0x??    SCMPLE    vres    var   var     A = B <= C
    0x??    SCMPGE    vres    var   var     A = B >= C
    0x??    SCMPLT    vres    var   var     A = B < C
    0x??    SCMPGT    vres    var   var     A = B > C
    0x??    SCMPBEG   vres    var   var     A = B =^ C
    0x??    SCMPEND   vres    var   var     A = B =$ C
    0x??    SCMPSET   vres    var   var     A = B in C
    0x??    RMATCH    vres    var   var     A = B =~ C

#### Control Ops

    Opcode  Mnemonic  A       D             Description
    --------------------------------------------------------------------------------------------
    0x??    JMP       -       pc            Unconditionally jump to $pc
    0x??    CONDBR    -       pc            Conditionally jump to $pc if prior CMP was true
    0x??    EXIT      -       imm           End program with given boolean status code

#### Native Call Ops

    Opcode  Mnemonic  A       B     C       Description
    --------------------------------------------------------------------------------------------
    0x??    FUNCTION  -       vbase imm     Invoke native func, returns an integer
    0x??    FUNCTIONI vres    vbase imm     Invoke native func, returns an integer
    0x??    FUNCTIONS vres    vbase imm     Invoke native func; returns a string
    0x??    HANDLER   -       vbase imm     Invoke native handler (can cause program to exit)

Native functions and handlers must have their parameters stored in consecutive registers directly after
the function's ID.

The consecuritve register array layout can be described as follows:

      vbase+0     vbase+1   ...   vbase+imm
    +-----------+---------+-----+-----------+
    | NativeID  | arg1    | ... | argN      |
    +-----------+---------+-----+-----------+

##### Examples:

We assume that the native function ID is stored in register `0x11`.

- `0x001100??` - native call, returns nothing and takes no args
- `0x001122??` - native call, returns a result to register `0x22`, takes no args
- `0x011122??` - native call, returns a result, takes one arg into register `0x33`
- `0x041122??` - native call, returns a result, takes 4 args starting from register `0x12` to `0x15` including.

