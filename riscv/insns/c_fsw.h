require_extension(EXT_ZCF);
require_fp;
MMU.store<uint32_t>(RVC_RS1S + insn.rvc_lw_imm(), static_cast<freg_t>(RVC_FRS2S).v[0]);
