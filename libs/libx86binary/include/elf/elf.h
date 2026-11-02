#include <elf.h>
#ifndef KRACK_ELF_H
#define KRACK_ELF_H
#include "../../../../utils/types.h"
#include "../../../../utils/include/llist.h"

#define ELF_STATUS		int
#define ELF_OK			0
#define ELF_BAD_PHDR	22

enum ElfArchType {
	ELF_X86		= 32,
	ELF_X86_64	= 64,
};

enum ElfExecType {
	ElfNone=0,
	ElfExec,
	ElfObj,
	ElfPIE,
	ElfDriver,
	ElfDum,
};

#define ELF_MAGIC(ptr) ({										\
		__u8 *b = (__u8*)(ptr);									\
		!!(b[0]=='\x7f' && b[1]=='E' && b[2]=='L' && b[3]=='F');\
	})

#define ElfUnion(t, e)				\
		union {						\
			Elf64_##t	*e##64;		\
			Elf32_##t	*e##32;		\
			void		*e;			\
		}

typedef struct _elfobj_t {
	struct {
		char		*name;
		__u64		size;
	} f;
	char			*runpath;

	ElfArchType		arch;
	ElfExecType		type;
	union {
		void 		*map;
		__u8		*mem;
		ElfUnion(Ehdr, ehdr);
	};
	__u64 base_virt;

	ElfUnion(Phdr,		phdr);
	ElfUnion(Shdr, 		shdr);
	LLIST_OF(ElfUnion(Rela,	rela))		ll_rela;
	LLIST_OF(ElfUnion(Rel,	rel))		ll_rel;
	LLIST_OF(ElfUnion(Sym,	sym))		ll_sym;
	LLIST_OF(ElfUnion(Sym,	dynsym))	ll_dynsym;
	LLIST_OF(ElfUnion(Nhdr,	note))		ll_note;
	LLIST_OF(char*)			ll_libs;

	struct {
		char		*interp;
		char		*strtab;
		ElfUnion(Shdr,	plt);
		ElfUnion(Shdr,	got);
		ElfUnion(Shdr,	got_plt);
		ElfUnion(Sym,	symtab);
		ElfUnion(Sym,	dynsym);
		ElfUnion(Dyn,	dynamic);
		ElfUnion(Shdr,	str_shdr);
	} e_usr;
} elfobj_t;


#define with_ElfEhdr(_elfobj_, __ehdr__, __VA_ARGS__) do {					\
		if (!!(_elfobj_)->IsX64()) {										\
			auto __ehdr__=decltype((_elfobj_)->ehdr64)((_elfobj_)->ehdr);	\
			__VA_ARGS__;													\
		} else if (!!(_elfobj_)->IsX86()) {									\
			auto __ehdr__=decltype((_elfobj_)->ehdr32)((_elfobj_)->ehdr);	\
			__VA_ARGS__;													\
		} else {															\
			assert(0);														\
		}																	\
	} while (0);

#define ELF_PHDR_ALIGN(ph)	((ph)->p_vaddr + ((ph)->p_align-1) & ~((ph)->p_align-1))

#define ELF_CASE(elf, a, b) ({					\
		__u64 _r3tn_ = 0;						\
		if ((elf)->IsX86()) _r3tn_=(elf)->##a;	\
		if ((elf)->IsX64()) _r3tn_=(elf)->##b;	\
		(_r3tn_);								\
	});

#define ELF_SIZEOF(elf, x) ({							\
		__u64 __r3t__=0;								\
		if ((elf)->IsX86()) __r3t__=sizeof(Elf32_##x);	\
    	if ((elf)->IsX64()) __r3t__=sizeof(Elf64_##x);	\
    	(__r3t__);										\
    });

#define AUXV_EACH(auxv, a, num)									\
		for (Elf64_auxv_t *a=(auxv); (void*)a < (void*)&(auxv)[(num)]; a++)

#define elfx64_each_ph(elf, p)									\
		for (Elf64_Phdr *p=(elf)->phdr64;						\
			(void*)p < (void*)(&(elf)->phdr64[(elf)->ehdr64->e_phnum]);	\
			p++)

#define elfx86_each_ph(elf, p)									\
		for (Elf32_Phdr *p=(elf)->phdr32;						\
			(void*)p < (void*)(&(elf)->phdr32[(elf)->ehdr32->e_phnum]);	\
			p++)

#define elfx64_each_sh(elf, s)									\
		for (Elf64_Shdr *s=(elf)->shdr64;						\
			(void*)s < (void*)(&(elf)->shdr64[(elf)->ehdr64->e_shnum]);	\
			s++)


#define elfx86_each_sh(elf, s)									\
		for (Elf32_Shdr *s=(elf)->shdr32;						\
			(void*)s < (void*)(&(elf)->shdr32[(elf)->ehdr32->e_shnum]);	\
			s++)

#define elfx64_each_dynamic(elf, dyn)					\
        for (Elf64_Dyn *dyn = (elf)->e_usr.dynamic64;	\
            dyn->d_tag != DT_NULL;						\
            dyn++)

#define elfx86_each_dynamic(elf, dyn)			\
        for (Elf32_Dyn *dyn = (elf)->dynamic;	\
            !!dyn->d_tag && !!dyn->d_un.d_val;	\
            dyn++)

#endif
