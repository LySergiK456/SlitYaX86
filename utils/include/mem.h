#include <assert.h>
#include "../types.h"

#define BINARY_ELF64    1
#define BINARY_PE32     2

#ifndef KRACK_MEM_H
#define KRACK_MEM_H

#if defined(_WIN32)
# define HANDLE_OK(h) (!(h) || (h) != INVALID_HANDLE_VALUE)
#endif

#define PAGE_SIZE           0x1000
#define PAGE_MASK           (PAGE_SIZE - 1)
#define PAGE_ALIGN_UP(x)    (((__u64)((x)) + PAGE_MASK) & ~PAGE_MASK)
#define PAGE_ALIGN_DOWN(x)  ((__u64)((x)) & ~PAGE_MASK)
#define ALIGN_UP(x, align)  (((__u64)((x)) + ((align)-1)) & ~((align)-1))

#define _contain_(a, sz, b)                                                    \
    ((__u64)(a) <= (__u64)(b) && (__u64)(b) < (__u64)(a) + (__u64)(sz))

#define _contains_(a, sz1, b, sz2)                                              \
    ((__u64)(a) <= (__u64)(b) && ((__u64)(a) + (__u64)(sz1)) >= ((__u64)(b) + (__u64)(sz2)))
    
#define _crossed_(a, sz1, b, sz2)                                               \
        (((__u64)(a) >= (__u64)(b) && (__u64)(a) < (__u64)(b) + (__u64)(sz2))   \
    ||  ( (__u64)(b) >= (__u64)(a) && (__u64)(b) < (__u64)(a) + (__u64)(sz1)))


#define MM_VMAP 2
#define MM_NONE 0
#define MM_HEAP 1

typedef struct {
    void *mem; // the order is important - this one (*mem) needs to be fixed at
               // off 0
    __u8 t;
    __u64 sz;
} mm_t;

typedef struct {
    void *mem;
    __u8 t;
    __u64 sz;
    __u64 off;
} mm_off_t;

struct offsz_t {
    __u64 off;
    __u64 sz;
};
#endif
