#include <sys/mman.h>
#include <map>
#include <vector>
#include "./include/elf/elf.hh"
extern "C" {
	#include "../libx86disass/disass.h"
};

namespace Assid
{
	struct lsd_entry_t {
		Elf64_Phdr	*phdr;
		__u8		*pMem;
		off_t		off;
		size_t		size;
		int			prot;
		lsd_entry_t() = default;
	};
};

namespace Assid {
class lsd25 {
	lsd25&							self{*this};
	Elf::ElfObj						elf;
	std::map<off_t, lsd_entry_t>	lsd_px;

public:
	lsd25(char *bin_path, bool modify=false); 

private:
	void split_phdr_px(off_t px_off, lsd_entry_t& px, Elf64_Phdr *ph);
	void _px_insert_(Elf64_Phdr *ph);
};
};

static int elf2sys_perm(int p_flags) {
	int p{0};
	if (p_flags&PF_R) p|=PROT_READ;
	if (p_flags&PF_W) p|=PROT_WRITE;
	if (p_flags&PF_X) p|=PROT_EXEC;
	return p;
}

namespace Assid
{
lsd25::lsd25(char *bin_path, bool modify)
	: elf(bin_path, nullptr, modify)
{
	prf(BGRN"[+]"CRST " Processing binary " BBLUE"\"%s\""CRST "\n", bin_path);
	elfx64_each_ph(&elf, ph) {
		prf(BGRN"[+]"CRST " adding phdr -> lsd_px/::map "
			"[" BRED"0x%lx"CRST "] { .size=" BRED"0x%lx"CRST " }\n",
			ph->p_offset, ph->p_filesz);

		_px_insert_(ph);
	}
}

void lsd25::split_phdr_px(off_t px_off, lsd_entry_t& px, Elf64_Phdr *ph) {
	__u64 low = std::min((__u64)px_off, (__u64)ph->p_offset);
    __u64 mid = std::max((__u64)px_off, (__u64)ph->p_offset);
    __u64 high= std::max((__u64)(px_off + px.size),
    					(__u64)(ph->p_offset + ph->p_filesz));
    std::vector<__u64> vec{low, mid, high};

    for (auto offset : vec) {
    	__u64 next{};
        for (auto& l : vec) {
            if (!next || (l > offset && l < next))
                next = l;
        }

        __u64 size = next - offset;
        if (    (offset == ph->p_offset && ph->p_offset < px_off)
            ||  (offset >= px_off + px.size
                &&  offset < ph->p_offset + ph->p_filesz))
        {
            lsd_px[offset] = (lsd_entry_t){
            	.phdr	= ph,	// nullptr
            	.pMem	= &elf.mem[offset],
            	.off 	= offset,
            	.size	= size,
            	.prot	= elf2sys_perm(ph->p_flags)
            };
        }
    }
}

void lsd25::_px_insert_(Elf64_Phdr *ph) {
	for (auto& [px_off, px] : lsd_px)
		if (!!_crossed_(px_off, px.off, ph->p_offset, ph->p_filesz)) {
		    split_phdr_px(px_off, px, ph);
		    return;
		}

	lsd_px[ph->p_offset] = (lsd_entry_t){
		.phdr	= ph,
		.pMem 	= &elf.mem[ph->p_offset],
		.off 	= ph->p_offset,
		.size	= ph->p_filesz,
		.prot 	= elf2sys_perm(ph->p_flags)
	};
}
};


int main() {
	auto p = Assid::lsd25(const_cast<char*>("/usr/bin/ssh"), false);
	// auto elf = Elf::ElfObj(const_cast<char*>("/usr/bin/ssh"), nullptr, true);
	// elfx64_each_ph(&elf, p)
	// {
	// 	if (!!(p->p_flags&PF_X)) {
	// 		for (int l=0; l<0x10; l++)
	// 			for (__u64 i=0; i<p->p_filesz;) {
	// 				instr_dat_t in;
	// 				if (init_instr(X86_64, &in, elf.mem + p->p_offset + i) == -1) {
	// 					i++;
	// 					continue;
	// 				}
	// 				// pr_in_str(&in);
	// 				i += in.in_sz;
	// 			}
	// 	}

	// 	prf("0x%lx (0x%lx)\n", p->p_offset, p->p_filesz);
	// }
}