#include "../include/rel_insn.hh"
#include "../include/lsd25.hh"
#include "../../libx86disass/disass.hpp"
extern "C" {
    #include "../../libx86disass/disass.h"
    #include "../include/rel_insn.h"
    #include "../../../utils/include/llist.h"
};

#define Y_each_tick(y, e)    ll_each_value((&(y)->ll_yayo), e)
#define Y_each_px(tick, e)   ll_each_value((&(tick)->ll_yayo_px), e)

#define x8664_each(in, i, src, _size_)                                                      \
        if (!!_size_) for (instr_dat_t in{0}; i < _size_ ; i += (!!in.in_sz) ? in.in_sz : 1)\
            if (init_instr((X86_64), (&in), (src + i)) != -1)

namespace lYayo {
    REL_INTERNAL static int _rel_insn_(struct y_aye_tic*, struct yayo_px*, mm_t&);
    REL_INTERNAL static const operand *operRIP(instr_dat_t*);
    REL_INTERNAL static __u64 ptrRIP(instr_dat_t*, struct yayo_px*, __u64);
    REL_INTERNAL static void *yayo_src_mem(struct y_aye_tic*, struct yayo_px*);
    static void _qmemcpy(void*, void*, __s64);
};

namespace lYayo
{
REL_INTERNAL
int _rel_insn_(
    struct y_aye_tic    *tic,
    struct yayo_px      *px,
    mm_t&               mm_out)
{
    char *ptr       = static_cast<char*>(mm_out.mem);
    __s64 g_align   = tic->out.align;
    void *src_mem   = yayo_src_mem(tic, px);

    __u64 i{};
    x8664_each(in, i, src_mem, px->dst_sz) {
        pr("!");
        __u64 off{px->dst_off + i};
        Y_each_px(px, qx) {
            if (qx->dst_off == off)
                if (_rel_insn_(tic, qx, mm_out) == -1)
                    return -1;
        }

        // off += pin_align();
        // off += lsd_align();
        // off += rel_align();
        ptr += off;

        __u64 base{off + tic->out.align};
        const operand *rip_op = operRIP(&in);
        if (!rip_op)
        {   /** Is not a .. PTR [ RIP ] instruction; just copy**/
            _qmemcpy(ptr, src_mem + i, in.in_sz);
            ptr += in.in_sz;
            continue;
        }

        __u64 orig_imm  = ptrRIP(&in, px, i);
        // __u64 imm       = orig_imm + ;

        switch (px->t) {
        case YAYO_PX_REL:
        {
        };
        break;
        default:
            return -1;
        }
        }
    }
};

namespace lYayo {
REL_INTERNAL
int Yayo::do_reloc(mm_t& mm_out) {
    Y_each_tick(this, tick)
        Y_each_px(tick, px) // struct lsd25_entry& lsd_out
            if (_rel_insn_(tick, px, mm_out) == -1) {
                return -1;
            }

    return 0;
}
};

namespace lYayo {
// Get source memory from a 'tick & a patch
REL_INTERNAL
static void *yayo_src_mem(struct y_aye_tic *tic, struct yayo_px *px) {
    switch (tic->t) {
    case YAYO_BINARY:
        return reinterpret_cast<void*>(
        &tic->bin.img->src.mem[
            ((!!px) ? px->orig_off : tic->bin.off)
        ]);

    case YAYO_SHELLCODE:
        return reinterpret_cast<void*>(
            (char*)tic->mm.mem + px->orig_off
        );
    }
    return NULL;
}
};

namespace lYayo
{   /** Instruction-releted Utils **/
// Get RIP operand from a C-struct instruction
static REL_INTERNAL
const operand *operRIP(instr_dat_t *in) {
    foreach_operand(oper, in)
        if (oper->rip && (oper->imm || oper->disp))
            return const_cast<const operand*>(oper);

    return nullptr;
}

// Get .. PTR [ RIP +/- ... *(?) ..] value
static REL_INTERNAL
__u64 ptrRIP(instr_dat_t *in, struct yayo_px *px, __u64 offset) {
    __u64 imm{}, in_off{(px->t==YAYO_PX_ADD)
                        ? px->dst_off : px->orig_off};
    get_rip_ptr_addr(
        in,
        reinterpret_cast<void*>(in_off + offset),
        &imm);

    return static_cast<__u64>(imm);
}

static
void _qmemcpy(void *_a, void *_b, __s64 n) {
    if (n<0) return;
    char *a=(char*)_a, *b=(char*)_b;
    while (!!n--) *a++ = *b++;
}
};

namespace lYayo
{
REL_API YayoTick *Yayo::_rel_init_(Binary_t *bin, __u64 orig_off, __u64 size) {
    struct y_aye_tic tic;
    memset(&tic, 0, sizeof(struct y_aye_tic));
    tic = (struct y_aye_tic){
        .t  = YAYO_BINARY,
        .bin = {
            .img = dynamic_cast<bin_t*>(bin),
            .off = orig_off,
            .sz = size
        }
    };
    if (llist_empty(&ll_yayo)) {
        llist_init(&ll_yayo, sizeof(struct y_aye_tic));
    }
    return static_cast<YayoTick*>(
        llist_push((_LL_TYPE_*)&ll_yayo, &tic)
    );
}
};

namespace lYayo {
    REL_API
    YayoPx *YayoTick::_rel_(
        enum yayo_px_t  t,
        __u64           orig_off,
        __u64           orig_size,
        __u64           dst_off,
        __u64           dst_size)
    {
        struct yayo_px px;
        memset(&px, 0, sizeof(struct yayo_px));
        px = (struct yayo_px){
            .t = t,
            .orig_off = orig_off, .dst_off = dst_off,
            .orig_sz = orig_size,.dst_sz = dst_size
        };
        if (llist_empty(&ll_yayo_px)) {
            llist_init(&ll_yayo_px, sizeof(struct yayo_px));
        }
        return static_cast<YayoPx*>(
            llist_push((_LL_TYPE_*)&ll_yayo_px, &px)
        );
    }
};