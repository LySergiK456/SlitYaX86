#include <string.h>
#include <assert.h>
#include <unicorn/unicorn.h>
#include "../../../utils/include/mem.hh"
extern "C" {
    #include "../../../utils/types.h"
    #include "../../../utils/include/utils.h"
    #include "../../../utils/include/mem.h"
};

#ifndef SYX86_MEM_HH
#define SYX86_MEM_HH
struct mem_map_t {
    __u64 addr;
    __u64 size;
    __u32 perm;
    char *name{};

    mem_map_t() = delete;
    mem_map_t(char *_name, __u64 _addr, __u64 _sz, __u32 _perm)
        : addr{_addr}, size{_sz}, perm{_perm} {
        if (!!_name) {
            name = reinterpret_cast<char *>(xmalloc(strlen(_name) + 1));
            memcpy(name, _name, strlen(_name) + 1);
        }
    }
};

struct shadow_mem_t {
    void *addr;
    __u64 size;
    __u32 perm;
    mem_map_t *uc_mem;

    // shadow_mem_t() = default;
    void *ptr(__u64 virt) {
        if (!addr || !uc_mem->addr)
            return NULL;
        if (uc_mem->addr + uc_mem->size == virt)
            return addr + size;

        __u64 v = (__u64)addr + (virt - uc_mem->addr);

        assert(_contain_(reinterpret_cast<__u64>(addr), size, v));
        return reinterpret_cast<void *>(v);
    }

    bool IsW() {
        // return perm & PAGE_EXECUTE_READWRITE || perm & PAGE_READWRITE;
        return perm & PAGE_EXEC;
    }
};

typedef enum {
    ErrOK = 0u,
    ErrNoDir = 1u,
    ErrNoFile = 2u,
    ErrCreateFile = 3u,
} perr_t;

const char *strerr(perr_t err);
uc_err _uc_init_x86_64_(uc_engine **uc);
__u32 FileExists(char *file);
__u32 DirExists(char *dir);
__u64 ptrArrSize(char **x);
int _write_mem(void *dst, void *src, __u64 size);
int _zero_memory(void *dst, __u64 size);
int _write_memset(void *dst, int ch, __u64 size);
int _write_qwordptr(void *ptr, __u64 v);
int _write_dwordptr(void *addr, __u32 v);
int _write_uint16(void *addr, __u16 v);
#endif
