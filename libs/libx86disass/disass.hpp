#ifndef X86DISASS_H
#define X86DISASS_H
#include "./uapi/cpp_uapi.hpp"
#include "./uapi/xtc.hpp"

#define _disasm(ptr, size, i, in)                               		\
    for (__u64 i=0; i < (size); i += (!!(in).in_sz ? (in).in_sz : 1), (in).init((char*)(ptr) + i))
    	
#endif