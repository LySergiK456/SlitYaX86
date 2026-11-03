#include <elf.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "../../include/elf/elf.hh"

#define IsOk(cond) !((cond) == -1)

namespace Elf
{
__s8 ElfObj::load_elf(bool map_shared) {
	__u32 flags = (!!map_shared) ? MAP_SHARED : MAP_PRIVATE;
	struct stat st{0};
	int fd = open(f.name, O_RDONLY);
	if (fd == -1) goto fail;

	if (stat(f.name, &st) == -1)
		goto fail;

	f.size = st.st_size;
	map = mmap(0, f.size, PROT_READ, flags, fd, 0);
	if (!map) goto fail;

	close(fd);
	return 0;
	fail:
	do {
		if (fd!=-1) close(fd);
		return -1;
	} while (0);
}

constexpr bool ElfObj::IsX64() {
	return ehdr64->e_ident[EI_CLASS] == ELFCLASS64;
}
constexpr bool ElfObj::IsX86() {
	return ehdr64->e_ident[EI_CLASS] == ELFCLASS32;
}
};

namespace Elf
{
ElfObj::ElfObj(char *file, char *symtab_dir, bool map_shared)
	: elfobj_t{}, Binary_t{}
{
	f.name = static_cast<char*>(xmalloc(strlen(file)+1));
	strncpy(f.name, file, strlen(file)+1);

	llist_init(&ll_rela,	sizeof(ll_rela.v));
	llist_init(&ll_rel,		sizeof(ll_rel.v));
	llist_init(&ll_sym, 	sizeof(ll_sym.v));
	llist_init(&ll_dynsym,	sizeof(ll_dynsym.v));
	llist_init(&ll_note, 	sizeof(ll_note.v));
	llist_init(&ll_libs, 	sizeof(char*));

	if (!IsOk(load_elf(map_shared)) && (ok=-1))
		return;

	if (!IsOk(parse_all()) && (ok=-1))
		return;

    bin->src.mem	= mem;
    bin->size   	= f.size;
	bin->entry_va = virtBase();
	if(!!IsX86() && (ok=-1)) {
		puts("X86 32bit not fucking supported!");
		return;
	}
}
};

namespace Elf
{
__s8 ElfObj::parse_all() {
	if (	!map || !ELF_MAGIC(map)
		||	(!IsX64() && !IsX86()))
	{
		puts("!MAP or Bad MAGIC or not an x86");
		return -1;
	}

	with_ElfEhdr(this, e, do {
		IsPIC = !!(e->e_type != ET_EXEC);
		if  (e->e_type == ET_NONE)
		{
			puts("[!] ELF type - None");
			return -1;
		}

		if (!!e->e_phnum) phdr = (void*)&mem[e->e_phoff];
		if (!!e->e_shnum) shdr = (void*)&mem[e->e_shoff];

		e_usr.str_shdr = (void*)&shdr64[e->e_shstrndx];
		e_usr.strtab = reinterpret_cast<char*>(&mem[e_usr.str_shdr64->sh_offset]);
	} while(0));

	elfx64_each_ph(self, ph)
	switch (ph->p_type) {
		case PT_DYNAMIC:
			e_usr.dynamic = reinterpret_cast<void*>(&mem[ph->p_offset]);
			parse_dynamic();
			break;

		case PT_LOAD:
			if (!!is_overflow(ph->p_offset + ph->p_filesz)) {
				printf("[!] overflowing phdr 0x%lx/0x%lx\n",
					ph->p_offset + ph->p_filesz,
					self->f.size);
				return -1;
			}
			__u32 pm = ph->p_flags;
			struct load_region reg = {
				.prot	= {
					.r=(pm&PF_R), .w=(pm&PF_W), .x=(pm&PF_X)
				},
				.off	= ph->p_offset,
				.size	= ph->p_filesz,
				.fillwith = '\xcc',
			};
			llist_push((_LL_TYPE_*)&ll_regions, &reg);
			break;
	}
	return 0;
}
};

namespace Elf
{
void ElfObj::parse_dynamic() {
	with_ElfEhdr(this, e, do {
		elfx64_each_dynamic(self, dyn)
		{
			switch (dyn->d_tag) {
				case DT_NEEDED:	break;
	            case DT_INIT:
	                bin->init_fn = (decltype(bin->init_fn))(
	                		self->base_virt
	                		+ (__u64)dyn->d_un.d_val);
	                break;
	           
	            case DT_FINI:
	                bin->fini_fn = (decltype(bin->fini_fn))(
	                		self->base_virt
	                		+ (__u64)dyn->d_un.d_val);
	                break;
	           
	            case DT_BIND_NOW:
	                bin->lazy_binding=-1;
	                break;

	            case DT_RPATH:
	            case DT_RUNPATH:
	                self->runpath = &self->e_usr.strtab[dyn->d_un.d_val];
	                break;
			}
		}
	} while(0));
}
};

bool Elf::dyn_is_ptr(Elf64_Dyn *dyn) {
	switch (dyn->d_tag) {
		case DT_STRTAB:
		case DT_SYMTAB:
		case DT_RELA:
		case DT_INIT:
		case DT_FINI:
		case DT_PLTGOT:
		case DT_INIT_ARRAY:
		case DT_FINI_ARRAY:
		case DT_REL:
		case DT_JMPREL:
		case DT_VERSYM:
		case DT_VERNEED:
		case DT_HASH:
		case DT_GNU_HASH:
			return true;
	}

	return false;
}

bool Elf::dyn_is_off(Elf64_Dyn *dyn) {
	switch (dyn->d_tag) {
		case DT_RUNPATH:
		case DT_SONAME:
		case DT_RPATH:
		case DT_NEEDED:
			return true;
	}

	return false;
}


namespace Elf
{	
__u64 ElfObj::vtof(__u64 virt) {
	elfx64_each_ph(this, ph)
		if (_contain_(ph->p_vaddr, ALIGN_UP(ph->p_memsz, ph->p_align), virt))
			return ph->p_offset + (virt - ph->p_vaddr);

	return -1;
}
__u64 ElfObj::ftov(__u64 off) {
	elfx64_each_ph(this, ph)
		if (_contain_(ph->p_offset, ph->p_filesz, off))
			return ph->p_vaddr + (off - ph->p_offset);

	return -1;
}
};

namespace Elf
{
bool ElfObj::is_overflow(__u64 off) {
	return (off >= self->f.size);
}
__u64 ElfObj::virtBase() {
	if (IsX64()) return self->ehdr64->e_entry;
	if (IsX86()) return self->ehdr32->e_entry;
}
};
