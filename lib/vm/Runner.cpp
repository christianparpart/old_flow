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
    program_(handler->program()),
    userdata_(nullptr),
    stringGarbage_()
{
    memset(data_, 0, sizeof(Register) * handler_->registerCount());
}

void Runner::operator delete (void* p)
{
    free(p);
}

String* Runner::createString(const std::string& value)
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

    #define toString(R) (*(String*) data_[R])
    #define toNumber(R)   ((Number) data_[R])

    #define instr(name) \
        l_##name: \
        disassemble(*pc, pc - code.data()); \
        ++ticks;

    #define next goto *ops[opcode(*++pc)]

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
        [Opcode::CALL] = &&l_call,
        [Opcode::HANDLER] = &&l_handler,
    };
    // }}}

    goto *ops[opcode(*pc)];

    // {{{ control
    instr (exit) {
        printf("exiting program. ran %lu instructions\n", ticks);
        return D != 0;
    }

    instr (jmp) {
        pc = code.data() + D;
        goto *ops[OP];
    }

    instr (condbr) {
        if (data_[A] != 0) {
            pc = code.data() + D;
            goto *ops[OP];
        } else {
            next;
        }
    }
    // }}}
    // {{{ copy
    instr (mov) {
        data_[A] = data_[B];
        next;
    }
    // }}}
    // {{{ debug
    instr (nticks) {
        data_[A] = ticks;
        next;
    }

    instr (ndumpn) {
        printf("regdump: ");
        for (int i = 0; i < B; ++i) {
            if (i) printf(", ");
            printf("r%d = %li", A + i, (int64_t)data_[A + i]);
        }
        if (B) printf("\n");
        next;
    }
    // }}}
    // {{{ numerical
    instr (imov) {
        data_[A] = D;
        next;
    }

    instr (nconst) {
        data_[A] = program->numbers()[D];
        next;
    }

    instr (nneg) {
        data_[A] = (Register) (-toNumber(B));
        next;
    }

    instr (nadd) {
        data_[A] = static_cast<Register>(toNumber(B) + toNumber(C));
        next;
    }

    instr (nsub) {
        data_[A] = static_cast<Register>(toNumber(B) - toNumber(C));
        next;
    }

    instr (nmul) {
        data_[A] = static_cast<Register>(toNumber(B) * toNumber(C));
        next;
    }

    instr (ndiv) {
        data_[A] = static_cast<Register>(toNumber(B) / toNumber(C));
        next;
    }

    instr (nrem) {
        data_[A] = static_cast<Register>(toNumber(B) % toNumber(C));
        next;
    }

    instr (nshl) {
        data_[A] = static_cast<Register>(toNumber(B) << toNumber(C));
        next;
    }

    instr (nshr) {
        data_[A] = static_cast<Register>(toNumber(B) >> toNumber(C));
        next;
    }

    instr (npow) {
        data_[A] = static_cast<Register>(powl(toNumber(B), toNumber(C)));
        next;
    }

    instr (nand) {
        data_[A] = data_[B] & data_[C];
        next;
    }

    instr (nor) {
        data_[A] = data_[B] | data_[C];
        next;
    }

    instr (nxor) {
        data_[A] = data_[B] ^ data_[C];
        next;
    }

    instr (ncmpeq) {
        data_[A] = static_cast<Register>(toNumber(B) == toNumber(C));
        next;
    }

    instr (ncmpne) {
        data_[A] = static_cast<Register>(toNumber(B) != toNumber(C));
        next;
    }

    instr (ncmple) {
        data_[A] = static_cast<Register>(toNumber(B) <= toNumber(C));
        next;
    }

    instr (ncmpge) {
        data_[A] = static_cast<Register>(toNumber(B) >= toNumber(C));
        next;
    }

    instr (ncmplt) {
        data_[A] = static_cast<Register>(toNumber(B) < toNumber(C));
        next;
    }

    instr (ncmpgt) {
        data_[A] = static_cast<Register>(toNumber(B) > toNumber(C));
        next;
    }
    // }}}
    // {{{ string
    instr (sconst) { // A = stringConstTable[D]
        data_[A] = (Register) &program->strings()[D];
        next;
    }

    instr (sadd) { // A = concat(B, C)
        data_[A] = (Register) createString(toString(B) + toString(C));
        next;
    }

    instr (ssubstr) { // A = substr(B, C /*offset*/, C+1 /*count*/)
        data_[A] = (Register) createString(toString(B).substr(data_[C], data_[C + 1]));
        next;
    }

    instr (scmpeq) {
        data_[A] = toString(B) == toString(C);
        next;
    }

    instr (scmpne) {
        data_[A] = toString(B) != toString(C);
        next;
    }

    instr (scmple) {
        data_[A] = toString(B) <= toString(C);
        next;
    }

    instr (scmpge) {
        data_[A] = toString(B) >= toString(C);
        next;
    }

    instr (scmplt) {
        data_[A] = toString(B) < toString(C);
        next;
    }

    instr (scmpgt) {
        data_[A] = toString(B) > toString(C);
        next;
    }

    instr (scmpbeg) {
        const auto& b = toString(B);
        const auto& c = toString(C);
        data_[A] = b.size() >= c.size() && strncmp(b.c_str(), c.c_str(), c.size()) == 0;
        next;
    }

    instr (scmpend) {
        const auto& b = toString(B);
        const auto& c = toString(C);
        data_[A] = b.size() >= c.size() && strcmp(b.c_str() + c.size() - c.size(), c.c_str()) == 0;
        next;
    }

    instr (scontains) {
        data_[A] = toString(B).find(toString(C)) != String::npos;
        next;
    }

    instr (slen) {
        data_[A] = toString(B).size();
        next;
    }

    instr (sprint) {
        printf("%s\n", toString(A).c_str());
        next;
    }
    // }}}
    // {{{ regex
    instr (sregmatch) { // A = B =~ C
        // TODO
        next;
    }

    instr (sreggroup) { // A = regex.match(B)
        // TODO
        next;
    }
    // }}}
    // {{{ conversion
    instr (s2i) { // A = atoi(B)
        data_[A] = strtoll(toString(B).c_str(), nullptr, 10);
        next;
    }

    instr (i2s) { // A = itoa(B)
        char buf[64];
        if (snprintf(buf, sizeof(buf), "%li", (int64_t) data_[B]) > 0) {
            data_[A] = (Register) createString(buf);
        } else {
            data_[A] = (Register) createString("");
        }
        next;
    }

    instr (surlenc) { // A = urlencode(B)
        // TODO
        next;
    }

    instr (surldec) { // B = urldecode(B)
        // TODO
        next;
    }
    // }}}
    // {{{ invokation
    instr (call) { // A = call(B[0], B.slice(1))
        Register id = toNumber(A);
        int argc = toNumber(B);
        Value* argv = &data_[C];

        Runtime::Callback* cb = handler_->program()->nativeFunction(id);
        cb->invoke(argc, argv, this);

        next;
    }

    instr (handler) {
        Register id = toNumber(A);
        int argc = toNumber(B);
        Value* argv = &data_[C];

        Runtime::Callback* cb = handler_->program()->nativeHandler(id);

        cb->invoke(argc, argv, this);

        if (argv[0] != 0) {
            return true;
        }

        next;
    }
    // }}}
}

} // namespace FlowVM
