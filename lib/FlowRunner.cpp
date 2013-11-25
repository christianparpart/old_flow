#include <flow/FlowRunner.h>
#include <flow/FlowProgram.h>
#include <flow/FlowInstruction.h>
#include <vector>
#include <utility>
#include <memory>
#include <new>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cmath>

std::unique_ptr<FlowRunner> FlowRunner::create(FlowProgram* program)
{
    FlowRunner* p = (FlowRunner*) malloc(sizeof(FlowRunner) + program->registerCount() * sizeof(uint64_t));
    new (p) FlowRunner(program);
    return std::unique_ptr<FlowRunner>(p);
}

FlowRunner::FlowRunner(FlowProgram* program) :
    program_(program),
    userdata_(nullptr),
    stringGarbage_()
{
    memset(data_, 0, sizeof(FlowRegister) * program->registerCount());
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

    #define toString(R) (*(std::string*)data_[R])
    #define toNumber(R) ((FlowNumber) data_[R])

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
        [FlowOpcode::MOV]       = &&l_mov,

        // numerical
        [FlowOpcode::IMOV]      = &&l_imov,
        [FlowOpcode::NCONST]    = &&l_nconst,
        [FlowOpcode::NNEG]      = &&l_nneg,
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

        // string op
        [FlowOpcode::SCONST]    = &&l_sconst,
        [FlowOpcode::SADD]      = &&l_sadd,
        [FlowOpcode::SSUBSTR]   = &&l_ssubstr,
        [FlowOpcode::SCMPEQ]    = &&l_scmpeq,
        [FlowOpcode::SCMPNE]    = &&l_scmpne,
        [FlowOpcode::SCMPLE]    = &&l_scmple,
        [FlowOpcode::SCMPGE]    = &&l_scmpge,
        [FlowOpcode::SCMPLT]    = &&l_scmplt,
        [FlowOpcode::SCMPGT]    = &&l_scmpgt,
        [FlowOpcode::SCMPBEG]   = &&l_scmpbeg,
        [FlowOpcode::SCMPEND]   = &&l_scmpend,
        [FlowOpcode::SCONTAINS] = &&l_scontains,
        [FlowOpcode::SLEN]      = &&l_slen,
        [FlowOpcode::SPRINT]    = &&l_sprint,

        // regex
        [FlowOpcode::SREGMATCH] = &&l_sregmatch,
        [FlowOpcode::SREGGROUP] = &&l_sreggroup,

        // conversion
        [FlowOpcode::S2I] = &&l_s2i,
        [FlowOpcode::I2S] = &&l_i2s,
        [FlowOpcode::SURLENC] = &&l_surlenc,
        [FlowOpcode::SURLDEC] = &&l_surldec,
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
l_mov:
    INSTR;
    data_[A] = data_[B];
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
    // {{{ numerical
l_imov:
    INSTR;
    data_[A] = D;
    next;

l_nconst:
    INSTR;
    data_[A] = program_->numbers()[D];
    next;

l_nneg:
    INSTR;
    data_[A] = (FlowRegister) (-toNumber(B));
    next;

l_nadd:
    INSTR2("+", B, C);
    data_[A] = static_cast<FlowRegister>(toNumber(B) + toNumber(C));
    next;

l_nsub:
    INSTR2("-", B, C);
    data_[A] = static_cast<FlowRegister>(toNumber(B) - toNumber(C));
    next;

l_nmul:
    INSTR2("*", B, C);
    data_[A] = static_cast<FlowRegister>(toNumber(B) * toNumber(C));
    next;

l_ndiv:
    INSTR2("/", B, C);
    data_[A] = static_cast<FlowRegister>(toNumber(B) / toNumber(C));
    next;

l_nrem:
    INSTR2("%", B, C);
    data_[A] = static_cast<FlowRegister>(toNumber(B) % toNumber(C));
    next;

l_nshl:
    INSTR2("<<", B, C);
    data_[A] = static_cast<FlowRegister>(toNumber(B) << toNumber(C));
    next;

l_nshr:
    INSTR2(">>", B, C);
    data_[A] = static_cast<FlowRegister>(toNumber(B) >> toNumber(C));
    next;

l_npow:
    INSTR2("**", B, C);
    data_[A] = static_cast<FlowRegister>(powl(toNumber(B), toNumber(C)));
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
    data_[A] = static_cast<FlowRegister>(toNumber(B) == toNumber(C));
    next;

l_ncmpne:
    INSTR2("!=", B, C);
    data_[A] = static_cast<FlowRegister>(toNumber(B) != toNumber(C));
    next;

l_ncmple:
    INSTR2("<=", B, C);
    data_[A] = static_cast<FlowRegister>(toNumber(B) <= toNumber(C));
    next;

l_ncmpge:
    INSTR2(">=", B, C);
    data_[A] = static_cast<FlowRegister>(toNumber(B) >= toNumber(C));
    next;

l_ncmplt:
    INSTR2("<", B, C);
    data_[A] = static_cast<FlowRegister>(toNumber(B) < toNumber(C));
    next;

l_ncmpgt:
    INSTR2(">", B, C);
    data_[A] = static_cast<FlowRegister>(toNumber(B) > toNumber(C));
    next;
    // }}}
    // {{{ string
l_sconst: // A = stringConstTable[D]
    INSTR;
    data_[A] = (FlowRegister) &program_->strings()[D];
    next;

l_sadd: // A = concat(B, C)
    INSTR;
    stringGarbage_.push_back(toString(B) + toString(C));
    data_[A] = (FlowRegister) &stringGarbage_.back();
    next;

l_ssubstr: // A = substr(B, C /*offset*/, C+1 /*count*/)
    INSTR;
    stringGarbage_.push_back(toString(B).substr(data_[C], data_[C + 1]));
    data_[A] = (FlowRegister) &stringGarbage_.back();
    next;

l_scmpeq:
    INSTR;
    data_[A] = toString(B) == toString(C);
    next;

l_scmpne:
    INSTR;
    data_[A] = toString(B) != toString(C);
    next;

l_scmple:
    INSTR;
    data_[A] = toString(B) <= toString(C);
    next;

l_scmpge:
    INSTR;
    data_[A] = toString(B) >= toString(C);
    next;

l_scmplt:
    INSTR;
    data_[A] = toString(B) < toString(C);
    next;

l_scmpgt:
    INSTR;
    data_[A] = toString(B) > toString(C);
    next;

l_scmpbeg:
    INSTR;
    {
        const auto& b = toString(B);
        const auto& c = toString(C);
        data_[A] = b.size() >= c.size() && strncmp(b.c_str(), c.c_str(), c.size()) == 0;
    }
    next;

l_scmpend:
    INSTR;
    {
        const auto& b = toString(B);
        const auto& c = toString(C);
        data_[A] = b.size() >= c.size() && strcmp(b.c_str() + c.size() - c.size(), c.c_str()) == 0;
    }
    next;

l_scontains:
    INSTR;
    data_[A] = toString(B).find(toString(C)) != std::string::npos;
    next;

l_slen:
    INSTR;
    data_[A] = toString(B).size();
    next;

l_sprint:
    INSTR;
    printf("%s\n", toString(A).c_str());
    next;
    // }}}
    // {{{ regex
l_sregmatch: // A = B =~ C
    INSTR;
    // TODO
    next;

l_sreggroup: // A = regex.match(B)
    INSTR;
    // TODO
    next;
    // }}}
    // {{{ conversion
l_s2i: // A = atoi(B)
    INSTR;
    data_[A] = strtoll(toString(B).c_str(), nullptr, 10);
    next;

l_i2s: // A = itoa(B)
    INSTR;
    {
        char buf[64];
        if (snprintf(buf, sizeof(buf), "%li", (int64_t) data_[B]) > 0) {
            stringGarbage_.push_back(buf);
        } else {
            stringGarbage_.push_back(std::string());
        }
        data_[A] = (FlowRegister) &stringGarbage_.back();
    }
    next;

l_surlenc: // A = urlencode(B)
    INSTR;
    // TODO
    next;

l_surldec: // B = urldecode(B)
    INSTR;
    // TODO
    next;
    // }}}
}
