#include "../include/lsd25.hh"
extern "C" {
    #include "../../../utils/include/llist.h"
//     #include "../../include/1d-lsd/yayo.h"
};
using MT    = MemConvType;
using Bin_t = Binary_t;

namespace Assid {
__u64 Lyss::LyssMemBase(Bin_t *binary, MT src_type, __u64 value, MT dst_type) {
    __u64 offset{};
    switch (src_type) {
        case MT::Virt:  offset = binary->vtof(value);   break;
        case MT::Off:   offset = value;                 break;
    }
    
    for (auto& [lsd_off, emm] : lsd_map)
        if (    emm.src.img == binary
            &&  !!_contain_(emm.src.off, emm.mm.sz, offset))
        {
            __u64 ret = lsd_off + (offset - emm.src.off);
            switch (dst_type) {
                case MT::Virt:  return mconv_ftov(binary, ret); break;
                case MT::Off:   return ret;                     break;
            }
        }

    return -1;
}

LSD_API void Lyss::push(Bin_t *img, enum LSD25_TYPE t, bin_hdr_t& v) {
    if (!v.mm.sz) return;

    for (auto& [off, entry] : lsd_map) {
        if (!!_crossed_(entry.dst.off, entry.dst.sz, v.off, v.mm.sz))
        {
            __u64 min = std::min(entry.dst.off, v.off);
            __u64 lvl = std::max(entry.dst.off, v.off);
            __u64 left= std::min(entry.dst.off + entry.dst.sz, v.off + v.mm.sz);
            __u64 rght = std::max(entry.dst.off + entry.dst.sz, v.off + v.mm.sz);
            std::vector<__u64> vec{min, lvl, left, rght};

            for (auto& _off_ : vec)
            {
                __u64 next{rght};
                for (auto i_off : vec) {
                    if (i_off > _off_ && i_off < next)
                        next = i_off;
                }

                __u64 size = next - _off_;
                if (    !!size && !lsd_map.count(_off_)
                    &&  (_off_ < rght && _off_ >= min && _off_ != left))
                {
                    lsd_map[_off_] = (struct lsd25_entry){
                        .t=(_off_ >= entry.dst.off && _off_ < entry.dst.off + entry.dst.sz) ? entry.t : t,
                        .mm={
                            .mem=reinterpret_cast<void*>(&img->src.mem[_off_]),
                            .sz =size
                        },
                        .src={ img, _off_, size },
                        .dst={ _off_, size }
                    };
                }
            }
            return;
        }
    }

    lsd_map[v.off] = (struct lsd25_entry){
        .t=t, .mm=v.mm, .src={img, v.off, v.mm.sz}, .dst={v.off, v.mm.sz}
    };
}
LSD_API void Lyss::push_new(enum LSD25_TYPE t, __u64 off, __u64 size) {
    push_ptr(&bin, t, new_lyss_entry(bin.src.map, off, size));
}
LSD_API void Lyss::push_ptr(Bin_t *img, enum LSD25_TYPE t, bin_hdr_t *&&v) {
    tofree.push_back(reinterpret_cast<void*>(v));
    push(img, t, *v);
}
};

namespace Assid {
    LSD_INTERNAL __u64 Lyss::orig_vtov(Bin_t *binary, __u64 v) { return LyssMemBase(binary, MT::Virt,  v, MT::Virt);}
    LSD_INTERNAL __u64 Lyss::orig_ftof(Bin_t *binary, __u64 v) { return LyssMemBase(binary, MT::Off,   v, MT::Off); }
    LSD_INTERNAL __u64 Lyss::orig_vtof(Bin_t *binary, __u64 v) { return LyssMemBase(binary, MT::Virt,  v, MT::Off); }
    LSD_INTERNAL __u64 Lyss::orig_ftov(Bin_t *binary, __u64 v) { return LyssMemBase(binary, MT::Off,   v, MT::Virt);}

    LSD_INTERNAL __u64 Lyss::mconv_vtov(Bin_t *binary, __u64 v) { return LyssMemConv(binary, MT::Virt, v, MT::Virt);}
    LSD_INTERNAL __u64 Lyss::mconv_ftof(Bin_t *binary, __u64 v) { return LyssMemConv(binary, MT::Off,  v, MT::Off); }
    LSD_INTERNAL __u64 Lyss::mconv_vtof(Bin_t *binary, __u64 v) { return LyssMemConv(binary, MT::Virt, v, MT::Off); }
    LSD_INTERNAL __u64 Lyss::mconv_ftov(Bin_t *binary, __u64 v) { return LyssMemConv(binary, MT::Off,  v, MT::Virt);}
};

namespace Assid
{
bin_hdr_t *new_lyss_entry(void *ptr, __u64 off, __u64 sz) {
    auto e = reinterpret_cast<bin_hdr_t*>(xmalloc(sizeof(bin_hdr_t)));
    e->mm = { (void*)((char*)ptr + off), MM_NONE, sz };
    e->off = off;
    return e;
}
};

namespace Assid {
LSD_API
std::vector<struct lsd25_entry*>
Lyss::VecLyssByType(enum LSD25_TYPE t, __u64 off) {
    std::vector<struct lsd25_entry *> retn;
    feach(t, [&](void *_, struct lsd25_entry *e) {
        if (e->src.off >= off) {retn.push_back(e);}
    });
    return retn;
}
LSD_API struct lsd25_entry *Lyss::LyssByType(enum LSD25_TYPE t, __u64 off) {
    struct lsd25_entry *retn=nullptr;
    feach(t, [&](void *_, struct lsd25_entry *e) {
        if (e->src.off >= off) { retn=e; return; };
    });
    return retn;
}
};

/** 4 Yayo **/
// LSD_API __u64 lsd_align(struct rel_yayo *ctx, __u64 offset) {
//     return offset;
// }
namespace Assid
{
// void Lyss::LyssAlignMap(__u64 off, __u64 sz) {
//     for (auto& [k, emm] : lsd_map) {
//         if (_contain_(k, emm.mm.sz, off)) {
//             lsd_map[k].mm.sz += sz;
//         } else if (k >= off) {
//             struct lsd25_entry e{std::move(emm)};
//             __u64 key{k+sz};

//             lsd_map.erase(k);
//             lsd_map[key] = std::move(e);
//         }
//         // DBG_FMT("/0x%lx\n", k);
//     }
// }
void Lyss::LyssAlignMap(__u64 off, __u64 sz) {
    std::map<__u64, struct lsd25_entry> out;
    for (auto& [k, emm] : lsd_map) out[k] = lsd_map[k];

    for (auto& [k, emm] : lsd_map) {
        if (_contain_(k, emm.mm.sz, off)) {
            out[k].mm.sz    += sz;
            out[k].dst.sz   += sz;
        } else if (k >= off) {
            out.erase(k);
            emm.dst.off = k+sz;
            out[k+sz]   = emm;
        }
    }
    lsd_map.erase(lsd_map.begin(), lsd_map.end());
    lsd_map = std::move(out);
}

LSD_API void Lyss::insert(enum LSD25_TYPE t, void *mem, __u64 size, __u64 offset) {
    for (auto& [off, sid_entry] : lsd_map)
        if (!!_contain_(off, sid_entry.dst.sz, offset)) {
            if (llist_empty(&sid_entry.px)) {
                llist_init(&sid_entry.px, sizeof(struct lsd25_entry));
            }

            auto px = (struct lsd25_entry){
                .raw_mem=mem, .dst={ .off=offset, .sz=size },
            };
            llist_push((_LL_TYPE_*)&sid_entry.px, &px);
            break;
        }

    LyssAlignBin(offset, size);
    LyssAlignMap(offset, size);
}

/**
 * Build the binary
**/
void lsd25::make(char *f) {
    mm_t mm{.mem=xmalloc(0x4000), .t=MM_HEAP, .sz=0x4000};
    yayo.do_reloc(mm);
    
    auto out = Util::outfile_t(f);

    printf(BGRN"[" BBLUE"lsd25" BGRN"]"CRST" - ~[+]~ Dumping to [%s]\n", f);
    for (auto& [off, e] : lyss->lsd_map) {
        if (!llist_empty(&e.px)) {
            ll_each_value(&e.px, px) {
                // printf("~> @0x%llx - size 0x%lx\n", px->dst.off, px->dst.sz);
                out.pwrite(px->raw_mem, px->dst.sz, px->dst.off);
            }
        }
        // printf("-> @0x%llx - size 0x%lx\n", off, e.mm.sz);
        out.pwrite(e.mm.mem, e.dst.sz, e.dst.off);
    }
}
};
