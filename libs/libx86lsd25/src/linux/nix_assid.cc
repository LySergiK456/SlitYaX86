#ifndef __linux__
__attribute__((constructor)) static void _die_now_() {
    static_assert(0, "This shall have been compiled for Linux");
}
#endif

#include "../../include/linux/assid.hh"
#include "../../include/lsd25.hh"
#include "../../../../utils/include/utils.hh"

namespace Assid {
    using MT    = MemConvType;
    using Bin_t = Binary_t;
};

namespace Assid
{
    NixLyss::NixLyss(NixLSD25 *xlsd, Elf::ElfObj *elf)
        : Lyss(xlsd, *dynamic_cast<Bin_t*>(elf))
    {
        void *pMem = elf->mem;
        /**
         *  Push Elf-Header */
        push_ptr(
            &bin,
            ELF_Ehdr,
            new_lyss_entry(
                pMem,
                0,
                sizeof(Elf64_Ehdr)));
        
        __u64 dyn_count{};
        __u64 dyn_off = ((__u64)elf->e_usr.dynamic - (__u64)pMem);
        elfx64_each_dynamic(elf, dyn) {
            push_ptr(
                &bin,
                ELF_Dyn,
                new_lyss_entry(
                    pMem,
                    dyn_off + (dyn_count++ * sizeof(Elf64_Dyn)),
                    sizeof(Elf64_Dyn)));
        }

        __u64 ph_ndx{};
        elfx64_each_ph(elf, ph)
        {   /** Push Each Elf_Phdr and it's content's **/
            push_ptr(
                &bin,
                ELF_Phdr,
                new_lyss_entry(
                    elf->mem,
                    elf->ehdr64->e_phoff + (ph_ndx++*sizeof(Elf64_Phdr)),
                    sizeof(Elf64_Phdr)));
            /**
             *  We got protections
            **/
            __u32 perms{};
            if (ph->p_flags&PF_R) perms |= LSD_R;
            if (ph->p_flags&PF_W) perms |= LSD_W;
            if (ph->p_flags&PF_X) perms |= LSD_X;

            push_ptr(
                &bin,
                static_cast<enum LSD25_TYPE>(perms),
                new_lyss_entry(pMem, ph->p_offset, ph->p_filesz));
        }

        __u64 sh_ndx{};
        elfx64_each_sh(elf, sec)
        {   /** Same for Sections. Header's and it's contents **/
            push_ptr(
                &bin,
                ELF_Shdr,
                new_lyss_entry(
                    pMem,
                    elf->ehdr64->e_shoff + (sh_ndx++*sizeof(Elf64_Shdr)),
                    sizeof(Elf64_Shdr)));

            push_ptr(
                &bin,
                LSD_NONE,
                new_lyss_entry(
                    pMem,
                    sec->sh_offset,
                    ALIGN_UP(sec->sh_size, sec->sh_addralign)));
        }
    }
};

namespace Assid {
LSD_INTERNAL
__u64 NixLyss::LyssSizeOf(enum LSD25_TYPE t) {
    Bin_t& s = bin;
    __u64 v = 0;
    switch (t) {
        case ELF_Ehdr:
            v = (s.IsX86()) ? sizeof(Elf32_Ehdr)
            :   (s.IsX64()) ? sizeof(Elf64_Ehdr) : -1;
            break;
        case ELF_Phdr:
            v = (s.IsX86()) ? sizeof(Elf32_Phdr)
            :   (s.IsX64()) ? sizeof(Elf64_Phdr) : -1;
            break;
        case ELF_Shdr:
            v = (s.IsX86()) ? sizeof(Elf32_Shdr)
            :   (s.IsX64()) ? sizeof(Elf64_Shdr) : -1;
            break;
        case ELF_Rela:
            v = (s.IsX86()) ? sizeof(Elf32_Rela)
            :   (s.IsX64()) ? sizeof(Elf64_Rela) : -1;
            break;
        case ELF_Rel:
            v = (s.IsX86()) ? sizeof(Elf32_Rel)
            :   (s.IsX64()) ? sizeof(Elf64_Rel) : -1;
            break;
        case ELF_Sym:
            v = (s.IsX86()) ? sizeof(Elf32_Sym)
            :   (s.IsX64()) ? sizeof(Elf64_Sym) : -1;
            break;
        case ELF_Dyn:
            v = (s.IsX86()) ? sizeof(Elf32_Dyn)
            :   (s.IsX64()) ? sizeof(Elf64_Dyn) : -1;
            break;
    }
    return v;
}
};

namespace Assid {
LSD_INTERNAL
__u64 NixLyss::LyssMemConv(Bin_t *binary, MT src_type, __u64 value, MT dst_type) {
    __u64 ret{-1};

    feach(ELF_Phdr, [&](Elf64_Phdr *_p, void *_) {
        __u64 base = (dst_type == MT::Virt) ? _p->p_vaddr : _p->p_offset;
        switch (src_type) {
        case MT::Virt:
            if (_contain_(_p->p_vaddr, ALIGN_UP(_p->p_memsz, _p->p_align), value)) {
                ret=base + (value - _p->p_vaddr);
                return;
            }
            break;
        case MT::Off:
            if (_contain_(_p->p_offset, _p->p_filesz, value)) {
                ret=base + (value - _p->p_offset);
                return;
            }
            break;
    }
    });

    return ret;
}

LSD_INTERNAL
void NixLyss::LyssAlignBin(__u64 off, __u64 sz) {
    auto e_ehdr = LyssByType(ELF_Ehdr, 0);
    assert(!!e_ehdr);
    void *img_ptr{e_ehdr->src.img->src.map};
    __u64 img_size{e_ehdr->src.img->size};

    __u32 perm{};
    __wr_init(
        reinterpret_cast<void*>(PAGE_ALIGN_DOWN(img_ptr)),
        PAGE_ALIGN_UP(img_size),
        &perm);


    auto ehdr = (Elf64_Ehdr*)e_ehdr->mm.mem;
    ehdr->e_entry = orig_vtov(&bin, ehdr->e_entry);
        
    if (ehdr->e_shoff > off) ehdr->e_shoff += sz;
    if (ehdr->e_phoff > off) ehdr->e_phoff += sz;

    feach(ELF_Dyn, [&](Elf64_Dyn *dyn, void *_) {
        if (!!Elf::dyn_is_ptr(dyn)) {
            printf("LOOKUP off=%#lx\n", off);

            for (auto& [base, e] : lsd_map)
                printf("RANGE [%#lx - %#lx]\n", base, base + e.mm.sz);
            
            prf("BEFOR [0x%lx]\n", dyn->d_un.d_ptr);
            dyn->d_un.d_ptr =
                orig_vtov(&bin, dyn->d_un.d_ptr);
            prf("AFTER [0x%lx]\n", dyn->d_un.d_ptr);
        }

        if (!!Elf::dyn_is_off(dyn))
            dyn->d_un.d_val = orig_ftof(&bin, dyn->d_un.d_val);
    });

    feach(ELF_Phdr, [&](Elf64_Phdr *_p, void *_) {
        if (_contain_(_p->p_offset, _p->p_filesz, off)) {
            _p->p_memsz     += sz;
            _p->p_filesz    += sz;
        } else if (_p->p_offset >= off) {
            _p->p_offset    += sz;
            _p->p_paddr     += sz;
            _p->p_vaddr     += sz;
        }
    });
    feach(ELF_Shdr, [&](Elf64_Shdr *sec, void *_) {
        if (_contain_(sec->sh_offset, sec->sh_size, off)) {
            sec->sh_size    += sz;
        } else if (sec->sh_offset >= off) {
            sec->sh_addr    += sz;
            sec->sh_offset  += sz;
        }
    });


    __wr_exit(
        reinterpret_cast<void*>(PAGE_ALIGN_DOWN(img_ptr)),
        PAGE_ALIGN_UP(img_size),
        perm);
}
};
