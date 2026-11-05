#ifdef __linux__
# include <linux/types.h>
# include <linux/fs.h>
# include <sys/ptrace.h>
# include <sys/syscall.h>
# include <sys/ioctl.h>
# include <sys/user.h>
# include <sys/wait.h>
# include <fcntl.h>
# include <string.h>
extern "C" {
	# include "../../../../utils/include/linux/proc_self_maps.h"
};
#endif
#include "../../include/linux/ptrace.hh"

namespace PlainRex
{
int ProcMapQuery(pid_t pid, struct procmap_query *q, __u64 addr) {
	memset(q, 0, sizeof(struct procmap_query));

	__u64 s_len		= strlen("/proc/65535/maps");
	char *strmaps	= static_cast<char*>(malloc(s_len+1));
	snprintf(strmaps, s_len+1, "/proc/%u/maps", pid);

	int fd = open(strmaps, O_RDONLY);	// make this into a class
	q->size			= sizeof(struct procmap_query);
	q->query_addr 	= addr;
	q->query_flags	= PROCMAP_QUERY_COVERING_OR_NEXT_VMA;

	int retn;
	retn = ioctl(fd, PROCMAP_QUERY, q);

#ifdef PLAINREX_NO_ERROR
		assert(!retn);	// ioctl
#endif
	if (!!retn)
		return -1;

	return 0;
}
};