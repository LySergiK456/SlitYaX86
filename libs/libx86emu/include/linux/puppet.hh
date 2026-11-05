#include <stdio.h>
#if defined(__linux__)
# include <linux/types.h>
# include <linux/fs.h>
# include <sys/ptrace.h>
# include <sys/syscall.h>
# include <sys/ioctl.h>
# include <sys/user.h>
# include <sys/wait.h>
# include <elf.h>
# include <fcntl.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include "../../../libx86disass/disass.hpp"
extern "C" {
	#include "../../../../utils/include/linux/proc_self_maps.h"
};

// int lsd25dbg(PlainRex::PTrace *pt);
