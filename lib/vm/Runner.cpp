#include <flow/vm/Runner.h>
#include <flow/vm/Handler.h>
#include <flow/vm/Program.h>
#include <flow/vm/Instruction.h>
#include <vector>
#include <utility>
#include <memory>
#include <new>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cmath>

namespace FlowVM {

std::unique_ptr<Runner> Runner::create(Handler* handler)
{
    Runner* p = (Runner*) malloc(sizeof(Runner) + handler->registerCount() * sizeof(uint64_t));
    new (p) Runner(handler);
    return std::unique_ptr<Runner>(p);
}

Runner::Runner(Handler* handler) :
    handler_(handler),
    userdata_(nullptr),
    stringGarbage_()
{
    memset(data_, 0, sizeof(Register) * handler_->registerCount());
}

void Runner::operator delete (void* p)
{
    free(p);
}

std::string* Runner::createString(const std::string& value)
{
    stringGarbage_.push_back(value);
    return &stringGarbage_.back();
}

bool Runner::run()
{
    const Program* program = handler_->program();
    const auto& code = handler_->code();
    register const Instruction* pc = code.data();
    uint64_t ticks = 0;

    #define OP opcode(*pc)
    #define A  operandA(*pc)
    #define B  operandB(*pc)
    #define C  operandC(*pc)
    #define D  operandD(*pc)
    #define next goto *ops[opcode(*++pc)]

    #define toString(R) (*(std::string*)data_[R])
    #define toNumber(R) ((Number) data_[R])

    #define INSTR do { ticks++; disassemble(*pc, pc - code.data()); } while (0)
    #define INSTR2(op, x, y) \
        do { \
            char buf[80]; \
            snprintf(buf, sizeof(buf), "%li %s %li", \
                    data_[x], op, data_[y]); \
            ticks++; \
            disassemble(*pc, pc - code.data(), buf); \
        } while (0)

    // {{{ jump table
    static const void* ops[] = {
        // control
        [Opcode::EXIT]      = &&l_exit,
        [Opcode::JMP]       = &&l_jmp,
        [Opcode::CONDBR]    = &&l_condbr,

        // debug
        [Opcode::NTICKS]    = &&l_nticks,
        [Opcode::NDUMPN]    = &&l_ndumpn,

        // copy
        [Opcode::MOV]       = &&l_mov,

        // numerical
        [Opcode::IMOV]      = &&l_imov,
        [Opcode::NCONST]    = &&l_nconst,
        [Opcode::NNEG]      = &&l_nneg,
        [Opcode::NADD]      = &&l_nadd,
        [Opcode::NSUB]      = &&l_nsub,
        [Opcode::NMUL]      = &&l_nmul,
        [Opcode::NDIV]      = &&l_ndiv,
        [Opcode::NREM]      = &&l_nrem,
        [Opcode::NSHL]      = &&l_nshl,
        [Opcode::NSHR]      = &&l_nshr,
        [Opcode::NPOW]      = &&l_npow,
        [Opcode::NAND]      = &&l_nand,
        [Opcode::NOR]       = &&l_nor,
        [Opcode::NXOR]      = &&l_nxor,
        [Opcode::NCMPEQ]    = &&l_ncmpeq,
        [Opcode::NCMPNE]    = &&l_ncmpne,
        [Opcode::NCMPLE]    = &&l_ncmple,
        [Opcode::NCMPGE]    = &&l_ncmpge,
        [Opcode::NCMPLT]    = &&l_ncmplt,
        [Opcode::NCMPGT]    = &&l_ncmpgt,

        // string op
        [Opcode::SCONST]    = &&l_sconst,
        [Opcode::SADD]      = &&l_sadd,
        [Opcode::SSUBSTR]   = &&l_ssubstr,
        [Opcode::SCMPEQ]    = &&l_scmpeq,
        [Opcode::SCMPNE]    = &&l_scmpne,
        [Opcode::SCMPLE]    = &&l_scmple,
        [Opcode::SCMPGE]    = &&l_scmpge,
        [Opcode::SCMPLT]    = &&l_scmplt,
        [Opcode::SCMPGT]    = &&l_scmpgt,
        [Opcode::SCMPBEG]   = &&l_scmpbeg,
        [Opcode::SCMPEND]   = &&l_scmpend,
        [Opcode::SCONTAINS] = &&l_scontains,
        [Opcode::SLEN]      = &&l_slen,
        [Opcode::SPRINT]    = &&l_sprint,

        // regex
        [Opcode::SREGMATCH] = &&l_sregmatch,
        [Opcode::SREGGROUP] = &&l_sreggroup,

        // conversion
        [Opcode::S2I] = &&l_s2i,
        [Opcode::I2S] = &&l_i2s,
        [Opcode::SURLENC] = &&l_surlenc,
        [Opcode::SURLDEC] = &&l_surldec,

        // invokation
        [Opcode::VOIDCALL] = &&l_voidcall,
        [Opcode::CALL] = &&l_call,
        [Opcode::HANDLER] = &&l_handler,
    };
    // }}}

    goto *ops[OP];

    // {{{ control
l_exit:
    INSTR;
    printf("exiting program. ran %lu instructions\n", ticks);
    return D != 0;

l_jmp:
    INSTR;
    pc = code.data() + D;
    goto *ops[OP];

l_condbr:
    INSTR;
    if (data_[A] != 0) {
        pc = code.data() + D;
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
l_nticks:
    INSTR;
    data_[A] = ticks;
    next;

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
    data_[A] = program->numbers()[D];
    next;

l_nneg:
    INSTR;
    data_[A] = (Register) (-toNumber(B));
    next;

l_nadd:
    INSTR2("+", B, C);
    data_[A] = static_cast<Register>(toNumber(B) + toNumber(C));
    next;

l_nsub:
    INSTR2("-", B, C);
    data_[A] = static_cast<Register>(toNumber(B) - toNumber(C));
    next;

l_nmul:
    INSTR2("*", B, C);
    data_[A] = static_cast<Register>(toNumber(B) * toNumber(C));
    next;

l_ndiv:
    INSTR2("/", B, C);
    data_[A] = static_cast<Register>(toNumber(B) / toNumber(C));
    next;

l_nrem:
    INSTR2("%", B, C);
    data_[A] = static_cast<Register>(toNumber(B) % toNumber(C));
    next;

l_nshl:
    INSTR2("<<", B, C);
    data_[A] = static_cast<Register>(toNumber(B) << toNumber(C));
    next;

l_nshr:
    INSTR2(">>", B, C);
    data_[A] = static_cast<Register>(toNumber(B) >> toNumber(C));
    next;

l_npow:
    INSTR2("**", B, C);
    data_[A] = static_cast<Register>(powl(toNumber(B), toNumber(C)));
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
    data_[A] = static_cast<Register>(toNumber(B) == toNumber(C));
    next;

l_ncmpne:
    INSTR2("!=", B, C);
    data_[A] = static_cast<Register>(toNumber(B) != toNumber(C));
    next;

l_ncmple:
    INSTR2("<=", B, C);
    data_[A] = static_cast<Register>(toNumber(B) <= toNumber(C));
    next;

l_ncmpge:
    INSTR2(">=", B, C);
    data_[A] = static_cast<Register>(toNumber(B) >= toNumber(C));
    next;

l_ncmplt:
    INSTR2("<", B, C);
    data_[A] = static_cast<Register>(toNumber(B) < toNumber(C));
    next;

l_ncmpgt:
    INSTR2(">", B, C);
    data_[A] = static_cast<Register>(toNumber(B) > toNumber(C));
    next;
    // }}}
    // {{{ string
l_sconst: // A = stringConstTable[D]
    INSTR;
    data_[A] = (Register) &program->strings()[D];
    next;

l_sadd: // A = concat(B, C)
    INSTR;
    data_[A] = (Register) createString(toString(B) + toString(C));
    next;

l_ssubstr: // A = substr(B, C /*offset*/, C+1 /*count*/)
    INSTR;
    data_[A] = (Register) createString(toString(B).substr(data_[C], data_[C + 1]));
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
        data_[A] = (Register) &stringGarbage_.back();
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
    // {{{ invokation
l_voidcall:
    INSTR;
    next;

l_call:
    INSTR;
    next;

l_handler:
    INSTR;
    next;

    // }}}
}

} // namespace FlowVM
