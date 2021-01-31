require_extension('C');
require_extension('D');
require_fp;
MMU.store_uint64(RVC_SP + insn.rvc_sdsp_imm(), static_cast<freg_t>(RVC_FRS2).v[0]);
