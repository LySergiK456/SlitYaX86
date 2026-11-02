#include <stdio.h>
#include "../types.h"

#ifndef KRACK_UTILS_H
#define KRACK_UTILS_H

#define prf		printf
#define pr		puts
#define px(x)	printf("0x%lx\n", (__u64)(x))

#ifdef __cplusplus
extern "C" {
#endif

void *xmalloc(__u64 size);
void hexdump(void *addr, __u64 sz);
__s8 _memcmp(void *a, void *b, __u64 size);
int __wr_init(void *addr, __u64 size, __u32 *dw);
int __wr_exit(void *addr, __u64 size, __u32 perm);

#ifdef __cplusplus
};
#endif
#endif
