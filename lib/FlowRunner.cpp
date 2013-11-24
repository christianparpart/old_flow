#include <flow/FlowRunner.h>
#include <flow/FlowProgram.h>
#include <flow/FlowInstruction.h>
#include <vector>
#include <utility>
#include <memory>
#include <new>
#include <cstdlib>
#include <cstdio>
#include <cmath>

std::unique_ptr<FlowRunner> FlowRunner::create(FlowProgram* program)
{
    FlowRunner* p = (FlowRunner*) malloc(sizeof(FlowRunner) + program->registerCount() * sizeof(uint64_t));
    new (p) FlowRunner(program);
    return std::unique_ptr<FlowRunner>(p);
}

void FlowRunner::operator delete (void* p)
{
    free(p);
}

bool FlowRunner::run()
{
    #define OP opcode(*pc)
    #define A  operandA(*pc)
    #define B  operandB(*pc)
    #define C  operandC(*pc)
    #define D  operandD(*pc)
    #define next goto *ops[opcode(*++pc)]

    #define INSTR do { icount++; disassemble(*pc, pc - program_->instructions().data()); } while (0)
    #define INSTR2(op, x, y) \
        do { \
            char buf[80]; \
            snprintf(buf, sizeof(buf), "%li %s %li", \
                    data_[x], op, data_[y]); \
            icount++; \
            disassemble(*pc, pc - program_->instructions().data(), buf); \
        } while (0)

    // {{{ jump table
    static const void* ops[] = {
        // control
        [FlowOpcode::EXIT]      = &&l_exit,
        [FlowOpcode::JMP]       = &&l_jmp,
        [FlowOpcode::CONDBR]    = &&l_condbr,

        // debug
        [FlowOpcode::NDUMPN]    = &&l_ndumpn,

        // copy
        [FlowOpcode::IMOV]      = &&l_imov,
        [FlowOpcode::NMOV]      = &&l_nmov,
        [FlowOpcode::NCONST]    = &&l_nconst,

        // binary: numerical
        [FlowOpcode::NADD]      = &&l_nadd,
        [FlowOpcode::NSUB]      = &&l_nsub,
        [FlowOpcode::NMUL]      = &&l_nmul,
        [FlowOpcode::NDIV]      = &&l_ndiv,
        [FlowOpcode::NREM]      = &&l_nrem,
        [FlowOpcode::NSHL]      = &&l_nshl,
        [FlowOpcode::NSHR]      = &&l_nshr,
        [FlowOpcode::NPOW]      = &&l_npow,
        [FlowOpcode::NAND]      = &&l_nand,
        [FlowOpcode::NOR]       = &&l_nor,
        [FlowOpcode::NXOR]      = &&l_nxor,
        [FlowOpcode::NCMPEQ]    = &&l_ncmpeq,
        [FlowOpcode::NCMPNE]    = &&l_ncmpne,
        [FlowOpcode::NCMPLE]    = &&l_ncmple,
        [FlowOpcode::NCMPGE]    = &&l_ncmpge,
        [FlowOpcode::NCMPLT]    = &&l_ncmplt,
        [FlowOpcode::NCMPGT]    = &&l_ncmpgt,
    };
    // }}}

    register const FlowInstruction* pc = program_->instructions().data();
    uint64_t icount = 0;

    goto *ops[OP];

    // {{{ control
l_exit:
    INSTR;
    printf("exiting program. ran %lu instructions\n", icount);
    return D != 0;

l_jmp:
    INSTR;
    pc = program_->instructions().data() + D;
    goto *ops[OP];

l_condbr:
    INSTR;
    if (data_[A] != 0) {
        pc = program_->instructions().data() + D;
        goto *ops[OP];
    } else {
        next;
    }
    // }}}
    // {{{ copy
l_imov:
    INSTR;
    data_[A] = D;
    next;

l_nmov:
    INSTR;
    data_[A] = data_[B];
    next;

l_nconst:
    INSTR;
    data_[A] = program_->numbers()[D];
    next;
    // }}}
    // {{{ debug
l_ndumpn:
    INSTR;
    printf("regdump: ");
    for (int i = 0; i < B; ++i) {
        if (i) printf(", ");
        printf("r%d = %li", A + i, (int64_t)data_[A + i]);
    }
    if (B) printf("\n");
    next;
    // }}}
    // {{{ binary numerical
l_nadd:
    INSTR2("+", B, C);
    data_[A] = data_[B] + data_[C];
    next;

l_nsub:
    INSTR2("-", B, C);
    data_[A] = data_[B] - data_[C];
    next;

l_nmul:
    INSTR2("*", B, C);
    data_[A] = data_[B] * data_[C];
    next;

l_ndiv:
    INSTR2("/", B, C);
    data_[A] = data_[B] / data_[C];
    next;

l_nrem:
    INSTR2("%", B, C);
    data_[A] = data_[B] % data_[C];
    next;

l_nshl:
    INSTR2("<<", B, C);
    data_[A] = data_[B] << data_[C];
    next;

l_nshr:
    INSTR2(">>", B, C);
    data_[A] = data_[B] >> data_[C];
    next;

l_npow:
    INSTR2("**", B, C);
    data_[A] = powl(data_[B], data_[C]);
    next;

l_nand:
    INSTR2("&", B, C);
    data_[A] = data_[B] & data_[C];
    next;

l_nor:
    INSTR2("|", B, C);
    data_[A] = data_[B] | data_[C];
    next;

l_nxor:
    INSTR2("^", B, C);
    data_[A] = data_[B] ^ data_[C];
    next;

l_ncmpeq:
    INSTR2("==", B, C);
    data_[A] = data_[B] == data_[C];
    next;

l_ncmpne:
    INSTR2("!=", B, C);
    data_[A] = data_[B] != data_[C];
    next;

l_ncmple:
    INSTR2("<=", B, C);
    data_[A] = data_[B] <= data_[C];
    next;

l_ncmpge:
    INSTR2(">=", B, C);
    data_[A] = data_[B] >= data_[C];
    next;

l_ncmplt:
    INSTR2("<", B, C);
    data_[A] = data_[B] < data_[C];
    next;

l_ncmpgt:
    INSTR2(">", B, C);
    data_[A] = data_[B] > data_[C];
    next;
    // }}}
}
