#ifndef YAYO_HH
#define YAYO_HH
#include "../../libx86binary/include/binary/binary.hh"
extern "C" {
	#include "../../libx86disass/disass.h"
	#include "../../../utils/include/llist.h"
	#include "../../../utils/types.h"
	#include "./rel_insn.h"
};

namespace RILkit
{
struct YayoPx
	: public yayo_px
{
	YayoPx() = default;
};

struct YayoTick
	: public y_aye_tic
{
	YayoTick() = default;

	// YayoPx *_rel_(enum yayo_px_t t, void *dat, __u64 og_off, __u64 dst_off, __u64 size);
	REL_API YayoPx *_rel_(enum yayo_px_t, __u64, __u64, __u64, __u64);
};

class Yayo {
	_rel_yayo_		y{};

public:
	Yayo() {
	    llist_init(&y.ll_yayo,	sizeof(struct y_aye_tic));
	    // llist_init(&y.ll_xtc_px,	sizeof(struct xtc_px));
	}

	REL_API YayoTick *_rel_init_(Binary_t *bin, __u64 orig_off, __u64 size);
};
};
#endif