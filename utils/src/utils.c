#if defined(__linux__)
# include <sys/mman.h>
#endif
#if defined(_WIN32)
# include <windows.h>
#endif

#include <assert.h>
#include "../include/utils.h"
#include "../include/mem.h"

void *xmalloc(__u64 size) {
    void *ptr = malloc(size);
    assert(!!ptr);
    memset(ptr, 0, size);
    return ptr;
}

void hexdump(void *addr, __u64 sz) {
    __u8 *ptr = (__u8 *)addr;

    for (__u64 i = 0; i <= sz; i++) {
        __u8 ch = ptr[i];

        if ((!(i % 0x10) && i) || i >= sz) {
            __u8 n = 0x10;

            if (i >= sz) {
                n = !(sz % 0x10) ? 0x10 : (sz % 0x10);
                for (int x = n; x < 0x10; x++) {
                    if (!(i % 4))
                        prf("  ");
                    prf("   ");
                }
            }

            prf("\t|\t");

            for (__u8 l = 0; l < n; l++) {
                __u8 c = ptr[(i - n) + l];
                prf("%c", isprint(c) ? c : '.');
            }

            pr("");
            if (i >= sz)
                return;
        }

        if (!i || !(i % 0x10))
            prf("0x%06x :\t", i);

        if (!(i % 4) && i && (i % 0x10))
            prf("| ");

        prf("%02x ", ch);
    }
}

int __wr_init(void *addr, __u64 size, __u32 *perm) {
#if defined(_WIN32)
    return VirtualProtect(PAGE_ALIGN_DOWN(addr), PAGE_ALIGN_UP(size),
        PAGE_READWRITE, perm);
#endif
#if defined(__linux__)
    *perm = UnixMemQuery(PAGE_ALIGN_DOWN(addr));
    return mprotect(addr, PAGE_ALIGN_UP(size), PROT_READ|PROT_WRITE);
#endif
}

int __wr_exit(void *addr, __u64 size, __u32 perm) {
#if defined(_WIN32)
    return VirtualProtect(PAGE_ALIGN_DOWN(addr), PAGE_ALIGN_UP(size),
        perm, NULL);
#endif
#if defined(__linux__)
    return mprotect(addr, PAGE_ALIGN_UP(size), perm);
#endif
}

__s8 _memcmp(void *a, void *b, __u64 size) {
    for (__u64 i = 0; i < size; i++)
        if (((char *)a)[i] != ((char *)b)[i])
            return -1;

    return 0;
}
