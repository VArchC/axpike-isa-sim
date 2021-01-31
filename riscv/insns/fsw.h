require_extension('F');
require_fp;
MMU.store_uint32(RS1 + insn.s_imm(), static_cast<freg_t>(FRS2).v[0]);
