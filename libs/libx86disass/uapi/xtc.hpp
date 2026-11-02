#include "../include/uapi.h"
// #include "./cpp_uapi.hpp"
#include <fcntl.h>
#include <map>
#include <list>

struct xtc_st {
    __u16   tabl_ndx;
    struct {
        __u8 rex, addr, oper;
    } pfx;

    struct {
        OperType    t;
        __u32       size;
    } op[4];
    __u8    op_count;
    __u8    sc[15];
    __u8    sc_len;
    instr *in;
};

class x_t_c {
public:
    x_t_c();
    __s8 get(__u8 *sc, insn_t& in, std::pair<OperType, __u64> op[4]);
    std::list<struct xtc_st> map_entry(char *s);
private:
    void insert(instr *in);
    void insert_insn(instr *in, struct xtc_st& x);
};


#define XTC_MAGIC     "\x55XTC\x55"
#define XTC_MAGIC_SZ  5

struct xtc_header {
    __u8    name[0x10];
    __u64   num;
};


#define rawT(T) typename std::remove_const<T>::type
using XMap = std::map<__u8 *, std::list<struct xtc_st>>;
class xtc_savefile {
    int     fd;
    __u64   off{};
public:
    xtc_savefile(const char *f)
        : fd{open(f, O_CREAT|O_WRONLY, 0644)}
    {
        wr(XTC_MAGIC, XTC_MAGIC_SZ);
    }

    void save(XMap& m);

    template<typename T> void wr(T *mem, __u64 size) {
        pwrite(fd, (void*)(const_cast<rawT(T)*>(mem)), size, off);
        off+=size;
    }
};

class xtc_readfile {
    int     fd;
    __u64   off{};
public:
    xtc_readfile(const char *f) : fd{open(f, O_RDONLY)} {}

    int load(XMap *m);
    __s64 rd(__u8 *mem, __u64 size) {
        __s64 r = pread(fd, mem, size, off);
        off += size;
        return r;
    }
};

