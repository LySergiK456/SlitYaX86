#ifndef KRACK_ELF_HH
#define KRACK_ELF_HH

#include "../binary/binary.hh"

extern "C" {
	#include "../../../../utils/include/utils.h"
	#include "./elf.h"
};

namespace Elf {
	class ElfObj
		: public elfobj_t, public Binary_t
	{
		/**	Self ptr's	**/
		elfobj_t		*self{ dynamic_cast<elfobj_t*>(this) };
		Binary_t		*bin{ dynamic_cast<Binary_t*>(this) };

	public:
		__s8		ok{0};

		ElfObj() = delete;
		ElfObj(char *file, char *symtab_dir, bool map_shared=false);

		__u64 virtBase();
		
		constexpr bool IsX64() override;	// x86_64 -> true
		constexpr bool IsX86() override;	// x86(32)-> true

	    __u64 vtof(__u64 v) override; // virt to offset
	    __u64 ftov(__u64 v) override; // offset to virt
		bool is_overflow(__u64 off);
	private:
		__s8 load_elf(bool map_shared=false);
		__s8 parse_all();
		void parse_dynamic();
	};
	
	bool dyn_is_ptr(Elf64_Dyn *dyn);
};
#endif