#include <elf.h>
#include <memory>
#include "./include/linux/assid.hh"
#include "../libx86binary/include/elf/elf.hh"

#define LSD_X64(x)	Elf64_##x
#define LSD_X86(x)	Elf32_##x

struct ObfuscA
	: Assid::AcidPass
{
    ObfuscA() = default;
    LSD_STATUS _init_(Assid::NixLSD25 *lsd);
};

static char *SC =
	"\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc"
	"\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc"
	"\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc"
	"\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc"
	"\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc"
	"\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc"
	"\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc"
	"\xcc\xcc";
// Header, Data, Code, lsd_map
LSD_STATUS ObfuscA::_init_(Assid::NixLSD25 *lsd) {
	auto phdr_st = lsd->lyss->VecLyssByType(ELF_Phdr);
	assert(!phdr_st.empty());
	__u64 i{};
	void  *tmp = xmalloc(phdr_st.size() * sizeof(Elf64_Phdr));
	for (auto *lsd_ph : phdr_st) {
		__u64 pt = ((Elf64_Phdr*)(lsd_ph->raw_mem))->p_type;
		if (pt == PT_PHDR || pt == PT_NULL) continue;
		memcpy(&((Elf64_Phdr*)tmp)[i++], lsd_ph->raw_mem, sizeof(Elf64_Phdr));
	}

	__u64 l{phdr_st.size()-1};
	for (auto* lsd_ph : phdr_st) {
		__u64 pt = ((Elf64_Phdr*)(lsd_ph->raw_mem))->p_type;
		if (pt == PT_PHDR || pt == PT_NULL) continue;
		__u32 perm{};
		__wr_init(
			reinterpret_cast<void*>(PAGE_ALIGN_DOWN(lsd_ph->mm.mem)),
			PAGE_SIZE,
			&perm);
		
		memcpy(lsd_ph->mm.mem, &((Elf64_Phdr*)tmp)[--l], sizeof(Elf64_Phdr));
		
		__wr_exit(
			reinterpret_cast<void*>(PAGE_ALIGN_DOWN(lsd_ph->mm.mem)),
			PAGE_SIZE,
			perm);
	}

	__u64 pt_load_offset{0};
	for (auto *lsd_ph : phdr_st) {
		auto *ph = reinterpret_cast<Elf64_Phdr*>(lsd_ph->raw_mem);
		if (ph->p_type == PT_LOAD
			&& (	!pt_load_offset
				||	(ph->p_offset + ph->p_filesz > pt_load_offset)))
		{
			pt_load_offset = ph->p_offset + ph->p_filesz;
		}
	}

	lsd->lyss->insert(
		static_cast<enum LSD25_TYPE>(LSD_R|LSD_X),
		(void*)SC,	// The fucking shellcode which shall reide as a Phdr Entry
		56,			// SizeOf Shellcode
		pt_load_offset);

	return LSD_OK;
	lsd->parse<ELF_Shdr>([&](__u64 off, struct lsd25_entry& e) {
		auto sec = reinterpret_cast<Elf64_Shdr*>(e.mm.mem);
		__u32 perm{};
		__wr_init(
			reinterpret_cast<void*>(PAGE_ALIGN_DOWN(sec)),
			PAGE_SIZE,
			&perm);
		
		memset(sec, 0, sizeof(Elf64_Shdr));
		
		__wr_exit(
			reinterpret_cast<void*>(PAGE_ALIGN_DOWN(sec)),
			PAGE_SIZE,
			perm);
	});

	auto *ehdr = ((Elf64_Ehdr*)lsd->lyss->LyssByType(ELF_Ehdr)->raw_mem);
	__u32 perm{};
	__wr_init(
		reinterpret_cast<void*>(PAGE_ALIGN_DOWN(ehdr)),
		PAGE_SIZE,
		&perm);

	ehdr->e_shnum=0;
	ehdr->e_shoff=0;
	
	__wr_exit(
		reinterpret_cast<void*>(PAGE_ALIGN_DOWN(ehdr)),
		PAGE_SIZE,
		perm);

	lsd->parse<ELF_Phdr>([&](__u64 off, struct lsd25_entry& e) {
		auto p = reinterpret_cast<Elf64_Phdr*>(e.mm.mem);
		switch (p->p_type) {
		case PT_NULL:
		case PT_LOAD:
		case PT_PHDR:
		case PT_DYNAMIC:
		case PT_INTERP:
		case PT_GNU_PROPERTY:
		case PT_GNU_EH_FRAME:
		case PT_GNU_STACK:
		case PT_GNU_RELRO:	break;
		default:
			__u32 perm{};
			__wr_init(
				reinterpret_cast<void*>(PAGE_ALIGN_DOWN(p)),
				PAGE_SIZE,
				&perm);

			p->p_offset	= 0;//0xccccccccdeadbeef;
			p->p_filesz	= 0;//0xccccccccbeefcafe;
			p->p_vaddr	= 0;//0xcccccc0ffeadd1c7;
			p->p_memsz	= 0;//0xacab4deadccccccc;
			
			__wr_exit(
				reinterpret_cast<void*>(PAGE_ALIGN_DOWN(p)),
				PAGE_SIZE,
				perm);
		break;
		}
	});
    return LSD_OK;
}

int main(int argc, char **argv) {
	auto pass = ObfuscA();

	auto elf = new Elf::ElfObj(const_cast<char*>("/usr/bin/ssh"), nullptr, false);
	auto lsd = Assid::NixLSD25(pass, elf);
	lsd.run();
	lsd.make("/tmp/hello.out");

	return 0;
}
