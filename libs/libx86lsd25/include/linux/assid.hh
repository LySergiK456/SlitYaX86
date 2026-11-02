#include <map>
#include <memory>
#include <vector>

#ifndef LSD25_LINUX_HH
#define LSD25_LINUX_HH
#include "../../../libx86binary/include/binary/binary.hh"
#include "../../../libx86binary/include/elf/elf.hh"
#include "../../../libx86disass/disass.hpp"
#include "../../../../utils/include/utils.hh"
#include "../../../../utils/include/mem.hh"
extern "C" {
    #include "../../../libx86binary/include/elf/elf.h"
    #include "../../../libx86binary/include/binary/binary.h"
    #include "../../../../utils/types.h"
    // #include "../yayo.h"
    #include "../lsd25.h"
};
#include "../lsd25.hh"

namespace Assid {
class NixLSD25;

struct NixLyss
    : public Lyss
{
    NixLyss() = delete;
    NixLyss(NixLSD25 *xlsd, Elf::ElfObj *elf);

    LSD_INTERNAL __u64 LyssMemConv(Binary_t*, MemConvType, __u64, MemConvType) override;
    LSD_INTERNAL __u64 LyssSizeOf(enum LSD25_TYPE t) override;
    LSD_INTERNAL void LyssAlignBin(__u64 off, __u64 sz) override;
};

class NixLSD25 : public lsd25 {
public:
    NixLSD25(AcidPass& _pass, Elf::ElfObj *elfobj)
        : lsd25(_pass, dynamic_cast<Lyss*>(new NixLyss(this, elfobj)))
    {}

    void run() override {
        pass._init_(this);
    }
};
};
#endif