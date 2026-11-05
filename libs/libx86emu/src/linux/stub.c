__attribute__((naked, noreturn)) int _start()
{	/** The sole purpose of Me, is to be used by ptrace **/
	asm volatile(
		"int3\n"
		"syscall\n"
		"int3\n");

	return ((void*(*)())0xdeadULL)();
}