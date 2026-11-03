#if defined(__linux__)
// # include "../../libx86binary/include/elf/elf.hh"
# include "./linux/assid.hh"
#endif
extern "C" {
    #include "../../../utils/types.h"
    // #include "../../libx86binary/include/binary/binary.h"
    // #include "./lsd25.h"
};
// #include "../../libx86binary/include/binary/binary.hh"

#ifndef LSD25_DOTS_HH
#define LSD25_DOTS_HH
namespace Assid::Dot {
	class lsd25;
};

namespace Assid::Dot {
	struct dot_analyzer {
		std::list<std::pair<struct lsd25_entry*, __u64>>	pt_load_caves;
	};
};

namespace Assid {
namespace Dot {
	class Dot {
		lsd25	*lsd;

	public:
		Dot(lsd25 *_lsd)
			: lsd{_lsd}
		{}
	};
};
};
#endif
