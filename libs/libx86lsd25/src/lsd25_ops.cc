#include "../include/lsd25.hh"
#include "../include/rel_insn.hh"
extern "C" {
    #include "../../../utils/include/llist.h"
};

namespace Assid {
namespace Dot {        
    Dot::Dot(Assid::lsd25 *_lsd)
        : lsd{_lsd} {}
};
};