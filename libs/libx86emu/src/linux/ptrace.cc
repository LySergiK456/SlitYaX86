#ifdef __linux__
# include <sys/ptrace.h>
# include <unistd.h>
# include <string.h>
# include <sys/signal.h>
# include <sys/user.h>
# include <sys/wait.h>
#endif
#include "../../include/linux/ptrace.hh"
#include "../../conf.hh"

namespace PlainRex
{
PTrace::PTrace(char *_file, int argc, char **args)
	: file{_file}, arg{ .count=argc }
{
	arg.v	= static_cast<char**>(malloc(arg.count*sizeof(char*)));
	arg.v[0]= file;
	memcpy(&arg.v[1], &args[1], arg.count*sizeof(char*));
}

int PTrace::ForkRun() {
	int status;
	if (!(child = fork()))
	{	/** Tracee Process **/
		ptrace(PTRACE_TRACEME, 0, NULL, NULL);
		execvp(arg.v[0], arg.v);
	}
	else
	{	/** Tracer Process **/
		while (waitpid(child, &status, 0) > 0) {
			if (WIFEXITED(status))
				break;

			if (!WIFSTOPPED(status))
				continue;

			if (WIFSIGNALED(status)) {
				int sig = WTERMSIG(status);
				if (sig == SIGINT) {
					PR_DBG("recv'd SIGINT");
					if (!!STOP_AT_SIGINT)
						break;
				}
				if (sig == SIGKILL) {
					PR_DBG("recv'd SIGKILL");
					break;
				}
			}

			int retn = 0;//lsd25dbg(this);
			if (retn == -1)
				break;
		}
	}

	return status;
}
};


namespace PlainRex
{
void PTrace::read_data(__u64 addr, void *out, __u64 size) {
	for (__u64 i=0; i<size; i += sizeof(long)) {
		long dw = ptrace(
			PTRACE_PEEKDATA,
			child,
			reinterpret_cast<void*>(addr+i),
			NULL);
		memcpy((char*)out + i, &dw, (size-i < sizeof(long)) ? size-i : sizeof(long));
	}
}
};

