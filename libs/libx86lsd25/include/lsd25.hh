#if defined(__linux__)
#include <elf.h>
#endif
#include <map>
#include <list>
#include <vector>
#include <memory>
extern "C" {
    #include "../../../utils/types.h"
    #include "../../libx86binary/include/binary/binary.h"
    #include "./lsd25.h"
    // #include "./yayo.h"
};
#if defined(__linux__)
# include "../../libx86binary/include/elf/elf.hh"
# include "./linux/assid.hh"
#endif
#include "../../libx86binary/include/binary/binary.hh"
#include "../../../utils/include/mem.hh"
// #include "../syx86/syx86.hh"

#ifndef LSD25_HH
#define LSD25_HH
#include "../../libx86disass/disass.hpp"
#include "../../../utils/include/utils.hh"

namespace Assid {
    class lsd25;
    class NixLSD25;   // Linux
    class WinLSD25;   // windows

    #if defined(__linux__)
        class ElfObj;
        class NixLyss;
    #endif
};

using MT    = MemConvType;
using Bin_t = Binary_t;

namespace Assid
{
/** Feed this to Lyss::push() **/
bin_hdr_t *new_lyss_entry(void *ptr, __u64 off, __u64 sz);
};

namespace Assid {
struct Lyss {
    struct {
        lsd25       *_lsd;
        #if defined(__linux__)
        NixLSD25    *nix_lsd;
        #endif
        #if defined(_WIN32)
        WinLSD25    *win_lsd;
        #endif
    };
    Bin_t&                  bin;
private:
    std::vector<void*>      tofree;
public:
    // std::map<struct lsd25_entry, struct rel_yayo>  m_yayo_entry;
    std::map<__u64, struct lsd25_entry>     lsd_map;

    /**
     * Linux    */
    Lyss() = delete;
    Lyss(lsd25 *_lsd_, Bin_t& _bin) : _lsd{_lsd_}, bin{_bin} {}
    #if defined(__linux__)
    Lyss(NixLSD25 *_lsd_, Bin_t& _bin) : nix_lsd{_lsd_}, bin{_bin} {}
    #endif
    #if defined(_WIN32)
    Lyss(WinLSD25 *_lsd_, Bin_t& _bin) : win_lsd{_lsd_}, bin{_bin} {}
    #endif

    virtual ~Lyss() {
        for (auto& e : tofree) delete e;
    }

    LSD_API void push_ptr(Bin_t *img, enum LSD25_TYPE t, bin_hdr_t *&&v);
    LSD_API void push_new(enum LSD25_TYPE t, __u64 off, __u64 size);
    LSD_API void push(Bin_t *img, enum LSD25_TYPE t, bin_hdr_t& v);

    template<typename Fn>
    int feach(enum LSD25_TYPE t, Fn fn) {
        return feach(t, nullptr, fn);
    }
    template<typename Fn>
    int feach(enum LSD25_TYPE t, struct lsd25_entry *v, Fn fn) {
        __u64 i{};
        try {
            for (auto& [_, e] : lsd_map) {
                if (e.t == t && ++i && !!e.raw_mem) {
                    if (!!v) *v = e;
                    for (__u64 off = 0, sz = LyssSizeOf(t); !!sz && off < e.mm.sz; off += sz) {
                        fn(static_cast<lambda_arg<decltype(fn)>>(e.mm.mem+off), &e);
                    }
                }
            }
        } catch (...) {}
        return i;
    }

    LSD_API void insert(enum LSD25_TYPE, void*, __u64, __u64);
    LSD_API struct lsd25_entry *LyssByType(enum LSD25_TYPE t, __u64 off=0);
    LSD_API std::vector<struct lsd25_entry*> VecLyssByType(enum LSD25_TYPE t, __u64 off=0);
    LSD_API std::map<__u64, struct lsd25_entry> &operator()() {
        return lsd_map;
    }
    LSD_INTERNAL
    virtual
    __u64 LyssMemConv(Bin_t*, MT, __u64, MT) = 0;
    __u64 LyssMemBase(Bin_t*, MT, __u64, MT);

    /**
     * Orignal Virt/Offset to Offset/Virt.
    **/
    LSD_INTERNAL __u64 orig_vtov(Bin_t*, __u64);
    LSD_INTERNAL __u64 orig_ftof(Bin_t*, __u64);
    LSD_INTERNAL __u64 orig_vtof(Bin_t*, __u64);
    LSD_INTERNAL __u64 orig_ftov(Bin_t*, __u64);

    LSD_INTERNAL __u64 mconv_vtov(Bin_t*, __u64);
    LSD_INTERNAL __u64 mconv_ftof(Bin_t*, __u64);
    LSD_INTERNAL __u64 mconv_vtof(Bin_t*, __u64);
    LSD_INTERNAL __u64 mconv_ftov(Bin_t*, __u64);

protected:
    LSD_INTERNAL
    virtual
    __u64 LyssSizeOf(enum LSD25_TYPE t) {
        assert(0);//, "You shall've implemented this!"
    };
    void LyssAlignMap(__u64 off, __u64 sz);
    LSD_INTERNAL
    virtual
    void LyssAlignBin(__u64 off, __u64 sz) {
        assert(0);//, "You Should've implement this!"
    }
    
private:
    friend class lsd25;
    friend class NixLyss;
    friend class WinLyss;
};
};

namespace Assid {
class AcidPass {
    // Util::MMap_t<lsd_oper_calback>  op_callbacks;
    lsd25       *lsd{nullptr};
    NixLSD25    *xlsd{nullptr};
    WinLSD25    *wlsd{nullptr};

  public:

  public:
    AcidPass() = default;
    virtual LSD_STATUS _init_(lsd25 *lsd) {
        assert(0);  // Shall have been implemented
        return LSD_OK;
    }

    // Linux
#if defined(__linux__)
    virtual LSD_STATUS _init_(NixLSD25 *lsd) {
        assert(0);  // Shall have been implemented
        return LSD_OK;
    }
#endif
    // Windows
#if defined(_WIN32)
    virtual LSD_STATUS _init_(WinLSD25 *lsd) {
        assert(0);  // Shall have been implemented
        return LSD_OK;
    }
#endif
    
    friend class lsd25;
    friend class NixLSD25;
    friend class WinLSD25;
};
};

namespace Assid {
class lsd25 : public lsd25_pack_t {
public:
    AcidPass&       pass;
    Lyss            *lyss;

    lsd25(AcidPass& _pass, Lyss *l)
        : pass{_pass}, lsd25_pack_t{0}, lyss{l}
    {
        llist_init(&ll_px,    sizeof(struct lsd25_entry));
        // llist_init(&ll_rel,   sizeof(struct rel_yayo));
        llist_init(&ll_cb,    sizeof(struct lsd_oper_cb));
    };
    virtual ~lsd25() = default;    
    virtual void run() {
        assert(0);  // shall be implemented
    };

    template<enum LSD25_TYPE T, typename Fn>
    void parse(Fn fn) {
        for (auto& [off, e] : lyss->lsd_map)
            if (e.t == T) fn(off, e);
    }

    void make(char *f);
};
};

// LSD_API __u64 lsd_align(struct rel_yayo *ctx, __u64 offset);
#endif

