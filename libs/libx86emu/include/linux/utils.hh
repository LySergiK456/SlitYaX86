#include <stdio.h>
#ifdef __linux__
# include <linux/types.h>
#endif

#ifndef SYX86_UTILS_HH
#define SYX86_UTILS_HH
#include "../../../libx86disass/disass.hpp"

constexpr const char *INPUT_BANNER =
		BGRN "[" BBLUE"lsd25"BGRN"]>"CRST;

#define PR_DBG(s)	printf("%s %s\n", INPUT_BANNER, (s))
#define PRF_DBG(s, ...) ({			\
		printf("%s ", INPUT_BANNER);\
		printf((s), __VA_ARGS__);	\
	})

#endif