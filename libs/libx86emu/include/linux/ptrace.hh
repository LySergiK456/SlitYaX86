#include <stdio.h>
#if defined(__linux__)
# include <linux/types.h>
# include <fcntl.h>
#endif
#include <stdlib.h>
#include "./utils.hh"

#ifndef SYX86_PTRACE_HH
#define SYX86_PTRACE_HH
namespace PlainRex
{
class PTrace {
	pid_t	child{};
	char	*file;
	struct {
		int		count;
		char	**v;
	} arg;

public:
	PTrace(char *_file, int argnum, char **args);
	int ForkRun();
	void read_data(__u64 addr, void *out, __u64 size);
};

int ProcMapQuery(pid_t, struct procmap_query*, __u64);
};
#endif
