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

LSD_STATUS ObfuscA::_init_(Assid::NixLSD25 *lsd) {
	// auto phdr_st = lsd->lyss->get_by_type(ELF_Phdr, 0);
	// assert(!!phdr_st);
	// lsd->lyss->insert(
	// 	LSD_NONE,	// Type
	// 	(void*)SC,	// The fucking shellcode which shall reide as a Phdr Entry
	// 	56,			// SizeOf Shellcode
	// 	0x1000,
	// 	(void*)phdr_st,	// struct lsd25_entry *
	// 	0,			// very first of all PHDR's
	// 	reinterpret_cast<void*>(phdr_st->mm->mem)	// Needs the fucking typecasting
	// );
	lsd->parse<ELF_Phdr>([&](__u64 off, struct lsd25_entry& e) {
		auto p = reinterpret_cast<Elf64_Phdr*>(e.mm.mem);
		switch (p->p_type) {
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

			p->p_offset	= 0xccccccccdeadbeef;
			p->p_filesz	= 0xccccccccbeefcafe;
			p->p_vaddr	= 0xcccccc0ffeadd1c7;
			p->p_memsz	= 0xacab4deadccccccc;
			
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
