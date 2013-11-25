## Flow Virtual Machine

### TODO

- maybe use 16-bit width register identification instead of 8-bit inside the opcode, effectively raising the register limit, would raise instruction size from 32-bit to 64-bit.
- support multi-branch instruction (merely like tableswitch in JVM)
- code: regex impl
- code: native function call
- code: native handler call
- code: multi branch instruction (design & impl)
- code: FlowAST-to-IR compiler to actually get this to life

### Strings

All strings in flow are immutable. So all string instructions do not disinguish between
strings from constant table, dynamically allocated strings, or strings as retrieved from
another virtual machine instruction (such as a native function call).

### Instruction Stream

The program is stored as an array (stream) of fixed-length 32-bit instructions.

The opcode always takes the least-significant 8-bit of an instruction, also determining the
interpretation of the higher 24 bits to one of the variations as described below.

A register is always represented as an 8-bit index to the register array, effectively limiting
the total number of registers to 256.

Registers does not necessarily require them to be located in a CPU hardware
but can also be represented as software array.

In the following tables, the values have the following meaning:

- *OP*: opcode
- *A*: first operand
- *B*: second operand
- *D*: second operand (alternative, immediate literal)
- *C*: third operand (illegal if *D* is used)

Instructions are represented as follows:

    0    8  16  24  32
    +----+---+---+---+      Instruction with no operands.
    | OP |           |
    +----+---+---+---+

    0    8  16  24  32
    +----+---+---+---+      Instruction with 1 operands.
    | OP | A |       |
    +----+---+---+---+

    0    8  16  24  32
    +----+---+---+---+      Instruction with 2 operands.
    | OP | A | B |   |
    +----+---+---+---+

    0    8  16  24  32
    +----+---+---+---+      Instruction with 3 operands.
    | OP | A | B | C |
    +----+---+---+---+

    0    8  16  24  32
    +----+---+---+---+      Instruction with 2 operands.
    | OP | A |   D   |      (second operand is usually an immediate literal, used as index
    +----+---+---+---+       to some constant table or to represent a 16bit short integer).

### Constants

Constants are all stored in a constant table, each type of constants in its own table.

- integer constants: 64-bit signed
- string constants: raw string plus its string length
- regular expression constants: defined as strings, but may be pre-compiled into dedicated AST for faster execution during runtime.

### Opcodes

#### Instruction Prefixes

 - *V* - variable
 - *N* - integer constant
 - *I* - immediate 16-bit integer literal
 - *S* - string constant

#### Instruction Operand Types

- *imm* - immediate literal values
- *num* - offset into the register array, cast to an integer.
- *str* - offset into the register array, cast to a string object.
- *var* - immediate offset into the register array, any type.
- *vres* - same as *var* but used by to store the instruction's result.
- *vbase* - same as *var* but used to denote the first of a consecutive list of registers.
- *pc* - jump program offsets, immedate offset into the program's instruction array

#### Debug Ops

    Opcode  Mnemonic  A       B             Description
    --------------------------------------------------------------------------------------------
    0x??    NDUMPN    vbase   imm           dumps register contents of consecutive registers [vbase, vbase+N]

#### Constant Ops

    Opcode  Mnemonic  A       B             Description
    --------------------------------------------------------------------------------------------
    0x??    NCLEAR    vres    -             set integer A to 0.
    0x??    SCLEAR    vres    -             set string A to empty string.

    Opcode  Mnemonic  A       D             Description
    --------------------------------------------------------------------------------------------
    0x??    IMOV      vres    imm           set integer A to immediate 16-bit integer literal D
    0x??    NCONST    vres    imm           set integer A to value at constant integer pool's offset D

#### Conversion Ops

    Opcode  Mnemonic  A       D             Description
    --------------------------------------------------------------------------------------------
    0x??    I2S       vres    num           A = itoa(D)
    0x??    S2I       vres    str           A = atoi(D)
    0x??    SURLENC   vres    str           A = urlencode(B)
    0x??    SURLDEC   vres    str           A = urldecode(B)

#### Unary Ops

    Opcode  Mnemonic  A       D             Description
    --------------------------------------------------------------------------------------------
    0x??    MOV       vres    var           A = D         /* raw register value copy */
    0x??    VNEG      vres    num           A = -D
    0x??    VNOT      vres    num           A = ~B
    0x??    SLEN      vres    str           A = strlen(D)

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

#### String Ops

    Opcode  Mnemonic  A       D             Description
    --------------------------------------------------------------------------------------------
    0x??    SCONST    vres    imm           A = stringConstantPool[D]

    Opcode  Mnemonic  A       B     C       Description
    --------------------------------------------------------------------------------------------
    0x??    SADD      vres    str   str     A = B + C
    0x??    SSUBSTR   vres    str   -       A = substr(B, C /* offset */, C + 1 /* count */)
    0x??    SCMPEQ    vres    str   str     A = B == C
    0x??    SCMPNE    vres    str   str     A = B != C
    0x??    SCMPLE    vres    str   str     A = B <= C
    0x??    SCMPGE    vres    str   str     A = B >= C
    0x??    SCMPLT    vres    str   str     A = B < C
    0x??    SCMPGT    vres    str   str     A = B > C
    0x??    SCMPBEG   vres    str   str     A = B =^ C
    0x??    SCMPEND   vres    str   str     A = B =$ C
    0x??    SCMPSET   vres    str   str     A = B in C
    0x??    SREGMATCH vres    str   regex   A = B =~ C
    0x??    SREGGROUP vres    num   -       A = regex_group(B /* regex-context offset */)

#### Control Ops

    Opcode  Mnemonic  A       D             Description
    --------------------------------------------------------------------------------------------
    0x??    JMP       -       pc            Unconditionally jump to $pc
    0x??    CONDBR    var     pc            Conditionally jump to $pc if int(A) evaluates to true
    0x??    EXIT      imm     -             End program with given boolean status code

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

