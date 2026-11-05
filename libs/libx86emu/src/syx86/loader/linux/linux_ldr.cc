#include <elf.h>
#include <memory>
#include "../../../../conf.hh"
#include "../../../../include/syx86.hh"
#include "../../../../include/loader/linux/linux_ldr.hh"

namespace x86Emu
{
	uc_err UnixLdr::init_stack(__u64 addr, __u64 sz) {
		return mmap(addr-sz, sz, UC_PROT_RW, "[stack]");
	}

	uc_err UnixLdr::init_proc_env() {
		uc_err err{UC_ERR_OK};
		err = init_stack(stack.top - stack.size, stack.size);
		if (!_uc_okay(err, ok))
			return err;

		return UC_ERR_OK;
	}
};

namespace x86Emu {
	UnixLdr::UnixLdr(int argc, char **argv)
		:	Ldr(argc, argv),
			sm(this, stack.top-stack.size)
	{
		__u64 off{0};
		uc_err err{UC_ERR_OK};
		if (!_uc_okay(_uc_init_x86_64_(&uc), ok)) {
			DBGS("[!] Failed to init Unicorn-Engine");
			return;
		}
		err = init_proc_env();
		if (!_uc_okay(err, ok)) {
			DBGS("[!] Failed to initialize process environment");
			return;
		}

		DBG_FMT("[+] Loading %s\n", argv[0]);
		main_elf = elfs_push(argv[0], ELF_SYMTAB_DIR);

		const char *interp_str = elf_interp(main_elf);
		if (!!interp_str) {
			interp.name	= const_cast<char*>(interp_str);
			interp.elf	= elfs_push(const_cast<char*>(interp_str), ELF_SYMTAB_DIR);
		} else {
			DBGS("[!] The program doesn't require a RT linker\n");
		}

		DBG_FMT("[+] Loading %s\n", main_elf->f.name);
		err = load_new_elf(main_elf, MAIN_ELF_BASE);
		if (!_uc_okay(err, ok)) {
			DBG_FMT("[-] Failed to load main Binary_t [%s]\n", main_elf->f.name);
			return;
		}

		DBG_FMT("[+] Loading %s\n", interp.name);
		err = load_new_elf(interp.elf, RTLD_ELF_BASE);
		if (!_uc_okay(err, ok)) {
			DBG_FMT("[-] Failed to load RTLD [%s]\n", interp.name);
			return;
		}

		// MKStack(&sm, args.num, args.v, nullptr, nullptr, 0);
	}
};

namespace x86Emu {
static bool ElfIsSupported(Elf::ElfObj& elf) {
	if (!elf.IsPIC) {
		DBG_FMT("[-] ELF is not Position-Independent \"%s\"",
			elf.f.name);
		return false;
	}
	if (!elf.IsX64()) {
		DBG_FMT("[-] ELF is not x86_64 - \"%s\"\n",
			elf.f.name);
		return false;
	}
	return true;
}
};

namespace x86Emu
{
	uc_err UnixLdr::load_new_elf(std::shared_ptr<Elf::ElfObj> elf, __u64 base) {
		if (!ElfIsSupported(*elf.get()))
			return UC_ERR_RESOURCE;

		elfx64_each_ph(dynamic_cast<elfobj_t*>(main_elf.get()), ph) {
			if (ph->p_type == PT_LOAD)
			{	/** Must be PIC **/
				__u64 elf_base = base + ph->p_vaddr;
			
				 __u32 p = 0;
		        if (ph->p_flags&PF_R) p |= UC_PROT_READ;
		        if (ph->p_flags&PF_W) p |= UC_PROT_WRITE;
		        if (ph->p_flags&PF_X) p |= UC_PROT_EXEC;

		        if (!!(elf_base & PAGE_MASK))
		        {	/** Interleaving PT_LOAD's **/
		        	elf_base = PAGE_ALIGN_DOWN(elf_base);
		        }

				mmap(elf_base,
					PAGE_ALIGN_UP(ph->p_memsz),
					p,
					elf->f.name);

				mmemcpy(base + ph->p_vaddr,
					(void*)&elf->mem[ph->p_offset],
					ph->p_filesz);
			}
		}
		return UC_ERR_OK;
	}
};

namespace x86Emu
{
const char* UnixLdr::elf_interp(std::shared_ptr<Elf::ElfObj> elf) {
	elfx64_each_ph(dynamic_cast<elfobj_t*>(elf.get()), p)
		if (p->p_type == PT_INTERP)
			return const_cast<const char*>((char*)&main_elf->mem[p->p_offset]);

	return nullptr;
}

std::shared_ptr<Elf::ElfObj>& UnixLdr::elfs_push(char *name, char *sym_dir) {
	return elfs.emplace_back(std::shared_ptr<Elf::ElfObj>{new Elf::ElfObj(name, sym_dir)});
}
};
