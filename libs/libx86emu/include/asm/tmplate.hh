#ifndef SYX86_ASM_TMPLATE_HH
#define SYX86_ASM_TMPLATE_HH
#define SYSCALL_OP			const_cast<char *>("\x0F\x05")
#define SYSCALL_LEGACY_OP	const_cast<char *>("\xCD\x2E")
bool IsSyscall(void *data) {
    return (!strncmp((char *)data, SYSCALL_OP, 2) ||
            !strncmp((char *)data, SYSCALL_LEGACY_OP, 2));
}

struct ASMStubTemplate {
	__u64	pre_int3_addr{};
	__u64	syscall_addr{};
	__u64 	post_int3_addr{};
private:
	Disass d;

public:
	ASMStubTemplate(__u64 base, void *dat, __u64 size)
	{
		__u8 *ptr{static_cast<__u8*>(dat)};
		d.iter(dat, size, [&](__u64 i, insn_t& in) {
			__u64 *int3_hand= (!syscall_addr)
						? &pre_int3_addr : &post_int3_addr;
			
			if (ptr[i] == 0xcc)
			{	/** Is INT3/{ 0xcc } **/
				*int3_hand = base + i;
				if (int3_hand == &post_int3_addr)
					throw DisassQuit;
			}

			if (!!IsSyscall(static_cast<void*>(ptr+i)))
			{	/** Is a syscall **/
				syscall_addr = base + i;
			}
		});
	}
};

#endif
