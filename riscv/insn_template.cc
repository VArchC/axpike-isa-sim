// See LICENSE for license details.

#include "insn_template.h"
#include "insns_approx/NAME.h"

reg_t rv32_NAME(processor_t* p, insn_t insn, reg_t pc)
{
  int xlen = 32;
  reg_t npc = sext_xlen(pc + insn_length(OPCODE));
  
  p->ax_control.cur_insn_id = INSN_NAME_ID;
  p->ax_control.insn = insn;
  p->ax_control.pc = pc;
  p->ax_control.xlen = xlen;
  p->ax_control.npc = npc;
  if (p->ax_control.EM[(INSN_NAME_ID << 2) | p->ax_control.prv] != NULL) {
    p->ax_control.EM[(INSN_NAME_ID << 2) | p->ax_control.prv](p, insn, pc, xlen, npc, NULL, NULL);
  }
  else {
    AxPIKE::Wrappers::Energy::__default__(p, insn, pc, xlen, npc, NULL, NULL);
  }

#if INSN_NAME_APPROX
  if (p->ax_control.IM[(INSN_NAME_ID << 2) | p->ax_control.prv] != NULL) {
    p->ax_control.IM[(INSN_NAME_ID << 2) | p->ax_control.prv](p, insn, pc, xlen, npc, NULL, NULL);
  }
  else {
    #include "insns/NAME.h"
  }
#else
  #include "insns/NAME.h"
#endif

  trace_opcode(p, OPCODE, insn);
  return npc;
}

reg_t rv64_NAME(processor_t* p, insn_t insn, reg_t pc)
{
  int xlen = 64;
  reg_t npc = sext_xlen(pc + insn_length(OPCODE));

  p->ax_control.cur_insn_id = INSN_NAME_ID;
  p->ax_control.insn = insn;
  p->ax_control.pc = pc;
  p->ax_control.xlen = xlen;
  p->ax_control.npc = npc;
  if (p->ax_control.EM[(INSN_NAME_ID << 2) | p->ax_control.prv] != NULL) {
    p->ax_control.EM[(INSN_NAME_ID << 2) | p->ax_control.prv](p, insn, pc, xlen, npc, NULL, NULL);
  }
  else {
    AxPIKE::Wrappers::Energy::__default__(p, insn, pc, xlen, npc, NULL, NULL);
  }

#if INSN_NAME_APPROX
  if (p->ax_control.IM[(INSN_NAME_ID << 2) | p->ax_control.prv] != NULL) {
    p->ax_control.IM[(INSN_NAME_ID << 2) | p->ax_control.prv](p, insn, pc, xlen, npc, NULL, NULL);
  }
  else {
    #include "insns/NAME.h"
  }
#else
  #include "insns/NAME.h"
#endif

  trace_opcode(p, OPCODE, insn);
  return npc;
}
