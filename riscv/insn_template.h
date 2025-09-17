// See LICENSE for license details.

#include "decode_macros.h"
#include "arith.h"
#include "mmu.h"
#include "softfloat.h"
#include "internals.h"
#include "specialize.h"
#include "tracer.h"
#include "v_ext_macros.h"
#include "debug_defines.h"
#include <assert.h>

namespace AxPIKE {
  namespace Wrappers {
    namespace Energy {
      void __default__(processor_t* p, insn_t insn, reg_t pc, int xlen, reg_t npc, source_t* source, void* data);
    }
  }
}
