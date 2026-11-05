#include "./include/linux/ptrace.hh"
#include "./conf.hh"

#ifndef __linux__
__attribute__((constructor)) void _die_now_() {
	static_assert(0, "This needs to be compiled for Linux");
}
#endif

namespace PR = PlainRex;

int main(int argc, char **argv) {
	auto pt = PR::PTrace(const_cast<char*>(SHMEM_STUB_OUT), argc-1, &argv[1]);
	pt.ForkRun();
	return 0;
}
