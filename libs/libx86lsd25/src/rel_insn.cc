#include "../include/rel_insn.hh"
extern "C" {
    #include "../../../utils/include/llist.h"
};

namespace RILkit
{
REL_API YayoTick *Yayo::_rel_init_(Binary_t *bin, __u64 orig_off, __u64 size) {
    struct y_aye_tic tic = {
        .t  =YAYO_BINARY,
        .bin={
            .img=dynamic_cast<bin_t*>(bin),
            .off=orig_off,
            .sz =size
        }
    };
    return reinterpret_cast<YayoTick*>(llist_push((_LL_TYPE_*)&y.ll_yayo, &tic));
}
};

namespace RILkit {
    REL_API
    YayoPx *YayoTick::_rel_(
        enum yayo_px_t  t,
        __u64           orig_off,
        __u64           orig_size,
        __u64           dst_off,
        __u64           dst_size)
    {
        struct yayo_px px = {
            .t=t,
            .orig_off=orig_off, .dst_off=dst_off,
            .orig_sz =orig_size,.dst_sz =dst_size
        };
        return reinterpret_cast<YayoPx*>(llist_push((_LL_TYPE_*)&ll_yayo_px, &px));
    }
};