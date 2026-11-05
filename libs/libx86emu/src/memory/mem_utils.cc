#include "../../include/memory.hh"
extern "C" {
    #include "../../../../utils/include/mem.h"

};
#include <assert.h>
#if defined(_WIN32)
# include <windows.h>
#endif
#if defined(__linux__)
# include <sys/mman.h>
extern "C" {
    # include "../../../../utils/include/linux/proc_self_maps.h"
};
#endif

const char *strerr(perr_t err) {
    switch (err) {
    case ErrOK:
        return "Success";

    case ErrNoDir:
        return "No such directory";

    case ErrNoFile:
        return "No such file";

    default:
        return "Unknown Error";
    }
}

__u64 ptrArrSize(char **x) {
    __u64 r=0;
    while(!!*x) { x++; r++; };
    return r;
}

#if defined(_WIN32)
__u32 FileExists(char *file) {
    __u32 dwAttr = GetFileAttributes(file);
    return (dwAttr != INVALID_FILE_ATTRIBUTES &&
            !(dwAttr & FILE_ATTRIBUTE_DIRECTORY));
}

__u32 DirExists(char *dir) {
    __u32 dwAttr = GetFileAttributes(dir);
    return (dwAttr != INVALID_FILE_ATTRIBUTES &&
            !!(dwAttr & FILE_ATTRIBUTE_DIRECTORY));
}
#endif
/**/

int _write_mem(void *dst, void *src, __u64 size) {
    __u32 dw;
    if (!dst || !src)
        return -1;

    if (!__wr_init(dst, size, &dw))
        return -1;
    memcpy(dst, src, size);
    if (!__wr_exit(dst, size, dw))
        return -1;

    return 0;
}

int _zero_memory(void *dst, __u64 size) {
    return _write_memset(dst, 0, size);
}

int _write_memset(void *dst, int ch, __u64 size) {
    __u32 dw;
    if (!dst)
        return -1;

    if (!__wr_init(dst, size, &dw))
        return -1;
    memset(dst, ch, size);
    if (!__wr_exit(dst, size, dw))
        return -1;

    return 0;
}
