// #include "../include/uapi.h"
#include "./cpp_uapi.hpp"
#include "./xtc.hpp"

static std::map<__u8 *, std::list<struct xtc_st>> xtc_map;

void x_t_c::insert_insn(instr *in, struct xtc_st& x) {
    insn_t instr(x.sc);
    if (instr.insn != in) return;
    
    x.in = in;
    x.sc_len = instr.size();

    for (__u8 i = 0; i < instr.OperCount(); i++) {
        x.op[i].t    = instr[i]->Type();
        x.op[i].size = instr[i]->size();
        x.op_count++;
    }

    if (!in->ops_count) {
        xtc_map[in->mnemonic].push_back(x);
    } else {
        for (__u8 i = 0; i < in->ops_count; i++) {
            xtc_map[in->ops[i].mnemonic].push_back(x);
        }
    }
}

void x_t_c::insert(instr *in) {
    for (__u8 l = 0; l < 0b111; l++) {
        __u8 pfx_i = 0;
        struct xtc_st x = {0};
        if (l&1) { x.pfx.rex    = (x.sc[pfx_i++] = 0x48); }
        if (l&2) { x.pfx.addr   = (x.sc[pfx_i++] = 0x67); }
        if (l&4) { x.pfx.oper   = (x.sc[pfx_i++] = 0x66); }
        __u64 op_i = pfx_i + _build_instr(in, &x.sc[pfx_i]);

        insert_insn(in, x);
        if (!!in->reg_op) {
            for (__u64 i = 0; i < 0xff; i++) {
                x.sc[op_i] = i;
                if (i&0b111 == 0b100) { // R/M is SIB
                    for (__u64 l = 0; l < 0xff; l++) {
                        x.sc[op_i+1] = l;
                        insert_insn(in, x);
                        x.sc[op_i+1] = 0;
                    }
                } else {
                    insert_insn(in, x);
                }
            }
        }
    }
}


x_t_c::x_t_c() {
    // if (xtc_map.size() > 0) return;
    // instr *in = NULL;
    // for_each_instr(X86_64, in) {
    //     if ((!in || !in->mnemonic || in->opcode1 == 0x0F
    //         || (in->prefix && !in->opcode1)))
    //         continue;
    //     insert(in);
    // }
    // xtc_savefile("/home/ok/.xtc_saved.bin").save(xtc_map);
    xtc_readfile("/home/ok/.xtc_saved.bin").load(&xtc_map);
}

std::list<struct xtc_st> x_t_c::map_entry(char *s) {
    __u64 len = strlen(s)+1;
    for (auto& [k, v] : xtc_map) if (!strncmp((char*)k, s, len)) return v;
    return {};
}

__s8 x_t_c::get(__u8 *sc, insn_t& in, std::pair<OperType, __u64> op[4]) {
    try {
        for (auto& x : map_entry(const_cast<char*>(in.Mnemo()))) {
            for (__u8 i = 0; i < in.OperCount(); i++) {
                if (static_cast<int>(std::get<0>(op[i]))) {
                //     if (in[i]->Type() != x.op[i].t)     break;
                //     if (in[i]->size() != x.op[i].size)  break;
                // } else {
                    if (x.op[i].t    != std::get<0>(op[i])) break;
                    if (x.op[i].size != std::get<1>(op[i])) break;
                }

                if (!(i+1 < in.OperCount())) {
                    memcpy(sc, x.sc, x.sc_len);
                    return x.sc_len;
                }
            }
        }
    } catch (...) {
        return -1;
    }
    return -1;
}

using XMap = std::map<__u8 *, std::list<struct xtc_st>>;
void xtc_savefile::save(XMap& m) {
    for (auto& [name, list] : m) {
        xtc_header hdr = { .num=list.size() };
        memcpy((void*)&hdr.name, (void*)name, strlen((char*)name)+1);
        wr(&hdr, sizeof(xtc_header));

        for (auto& e : list) {
            wr(&e, sizeof(struct xtc_st));
        }
    }
}

int xtc_readfile::load(XMap *m) {
    if (!m) return -1;

    __u8        buf[0x100];
    xtc_header  *hdr = (xtc_header*)buf;

    rd(buf, XTC_MAGIC_SZ);
    if (!memcmp(buf, XTC_MAGIC, XTC_MAGIC_SZ)) {
        for (;;) {
            if (!rd(buf, sizeof(xtc_header))) {
                return 0;
            }
            xtc_st      xst[hdr->num];
            __u8 *str = new __u8[ strlen((char*)&hdr->name)+1 ];
            memcpy(str, &hdr->name, strlen((char*)&hdr->name)+1);
            
            if (!rd((__u8*)&xst, hdr->num*sizeof(struct xtc_st))) {
                return 0;
            }

            for (__u64 l = 0; l < hdr->num; l++) {
                if (!xst[l].in) return 0;
                (*m)[str].push_back(xst[l]);
            }
        }
    }
    return 0;
}