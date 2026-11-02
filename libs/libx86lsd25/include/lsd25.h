#include "../../../utils/types.h"
#include "../../../utils/include/llist.h"
#include "../../libx86binary/include/binary/binary.h"
#include "../../libx86disass/disass.h"

#ifndef LSD25_API_H
#define LSD25_API_H
#define LSD_API
#define LSD_INTERNAL
typedef unsigned long long LSD_STATUS;

enum LSD_STATUS_CODE {
    LSD_OK,
    LSD_NULLPTR,
};

enum LSD25_BIN_TYPE {
    LSD_None    = 0,
    LSD_ELF32   = 1,
    LSD_ELF64   = 2,
    LSD_PE32    = 3
};
enum LSD25_TYPE {
    LSD_R   = 0x100,
    LSD_W  = 0x200,
    LSD_X  = 0x400,
    LSD_NONE= 0,
    /**
     *  ELF specific
    **/
    ELF_Ehdr=1,
    ELF_Phdr,
    ELF_Shdr,
    ELF_Rela,
    ELF_Rel,
    ELF_Sym,
    ELF_Dyn,
    ELF_GNU_Verneed,
    ELF_GNU_Versym,

    /**
     *  PE64 enums
    **/
    PE_DosHdr,
    PE_DosStub,
    PE_NtHdrs,
    PE_SecHdr,
    PE_Section,
    PE_DataDir,
    PE_DataDirExport,
    PE_DataDirImport,
    PE_DataDirRsrc,
    PE_DataDirExcept,
    PE_DataDirSec,
    PE_DataDirReloc,
    PE_DataDirDbg,
    PE_DataDirArch,
    PE_DataDirGlobalPtr,
    PE_DataDirTLS,
    PE_DataDirLoadConf,
    PE_DataDirBoundImp,
    PE_DataDirIAT,
    PE_DataDirDelayImp,
    PE_DataDirComDesc,
    PE_SEC_text,
    PE_SEC_data,
    PE_SEC_rdata,
    PE_SEC_bss,
    PE_SEC_idata,
    PE_SEC_edata,
    PE_SEC_reloc,
    PE_SEC_rsrc,
    PE_SEC_tls,
    PE_SEC_pdata,
    PE_SEC_xdata,
    PE_SEC_debug,
    PE_SEC_CRT,
    PE_SEC_gfids,
    PE_SEC_didat,
    PE_SEC_sxdata,
    PE_SEC_00cfg,
    PE_SEC_gehcont,
    PE_SEC_orpc,
    PE_SEC_arch,
    PE_SEC_drectve,
};

typedef int (*lsd_oper_calback)(instr_dat_t *insn, __u64 i, __u64 imm, instr_dat_t *out);

struct lsd_oper_cb {
    bin_t			*lsd_img;
    lsd_oper_calback    lsd_cb;
    operand             lsd_oper;
};

struct lsd25_entry {
    enum LSD25_TYPE     t;
    union {
        void        *raw_mem;
        mm_t        mm;
    };
    struct {
        bin_t       *img;
        __u64       off, sz;
    } src;
    struct {
        __u64      off, sz;
    } dst;
    LLIST_OF(struct lsd25_entry) px;
};

typedef struct {
    LLIST_OF(struct lsd25_entry)       ll_px;
    // LLIST_OF(struct rel_yayo)           ll_rel;
    LLIST_OF(struct lsd_oper_cb)  ll_cb;
} lsd25_pack_t;

// LSD_API __u64 lsd_align(struct rel_yayo *ctx, __u64 offset);
#endif