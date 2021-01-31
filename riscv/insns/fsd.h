require_extension('D');
require_fp;
MMU.store_uint64(RS1 + insn.s_imm(), static_cast<freg_t>(FRS2).v[0]);
