#ifndef KRACK_MEM_HH
#define KRACK_MEM_HH
enum class MemConvType : unsigned char
{
    Virt=1,
    Off =2,
};

enum MemPerm {
    PAGE_READ,
    PAGE_EXEC,
    PAGE_WRITE,
};
#endif
