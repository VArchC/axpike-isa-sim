require_extension(EXT_ZCF);
require_fp;
MMU.store<uint32_t>(RVC_SP + insn.rvc_swsp_imm(), static_cast<freg_t>(RVC_FRS2).v[0]);
