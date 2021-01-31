#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "acadp.h"

void create_setup_impl();
void create_wrappers_header();
void create_wrappers_impl();
void create_instruction_list();
void create_iface_file();

int main (int argc, char *argv[]) {
  int fi;
  if (argc < 2) {
    printf("Usage: %s <SPIKE insn_list.h> [list of adf files]\n", argv[0]);
    return EXIT_FAILURE;
  }

  acadp_load_instructions(argv[1]);
  acadp_init();

  for (fi = 2; fi < argc; fi++) {
    if(!acadp_load(argv[fi])) {
      printf("ADeLe: Could not open ADeLe Description File: %s.\n", argv[fi]);
      continue;
    }
    printf("ADeLe: Processing ADeLe Description File file %s:\n", argv[fi]);
    
    if (acadp_run() != 0) {
      printf("ADeLe: Parser terminated unsuccessfully for %s.\n", argv[fi]);
      acadp_destroy();
      acadp_destroy_instructions();
      return EXIT_FAILURE;
    }

    printf("\n");
  }

  create_setup_impl();
  create_wrappers_header();
  create_wrappers_impl();
  create_instruction_list();
  create_iface_file();

  acadp_destroy();
  acadp_destroy_instructions();

  return EXIT_SUCCESS;
}

void print_op(FILE* fp, AcadpOP* op, int instr, int approxid, const char* indent) {
  if (op != NULL) {
    int op_size = acadp_op_get_size(op);
    char* value;
    bool first = true;
    fprintf(fp, "%sOP.activateOP(%d, %d, {", indent, instr, approxid);
    for (int i = 0; i < op_size; i++) {
      value = acadp_op_get_value(op, i);
      if (value != NULL) {
        if (!first) {
          fprintf(fp, ", ");
        }
        fprintf(fp, "{\"%s\", %s}", acadp_op_get_key(i), value);
        first = false;
      }
    }
    fprintf(fp, "});\n");
  }
}

void create_setup_impl() {
  FILE *fp = fopen("axpike_setup.cc", "w");

  AcadpApprox* papprox = NULL;
  int approxid = 0;
  AcadpInstruction* pinstr = NULL;
  bool has_approx = false;
  AcadpGroup* pgroup = NULL;
  AcadpOP* op = NULL;
  char *genergy = NULL;

  fprintf(fp, "#include \"axpike_control.h\"\n");
  fprintf(fp, "#include \"axpike_wrappers.h\"\n\n");

  fprintf(fp, "void AxPIKE::Control::activateApprox(uint8_t approx_id) {\n");
  fprintf(fp, "\tswitch(approx_id) {\n");

  while (acadp_get_approx(&papprox)) {
    fprintf(fp, "\t\tcase 0x%02x: // %s\n", approxid, acadp_approx_get_name(papprox));
    while (acadp_get_instruction(&pinstr)) {
      if (acadp_approx_get_pre_behavior(papprox, pinstr->id) != NULL || acadp_approx_get_post_behavior(papprox, pinstr->id) != NULL || acadp_approx_get_alt_behavior(papprox, pinstr->id) != NULL) {
        fprintf(fp, "\t\t\tIM[%d].activateModel(%d, &Wrappers::Instruction::%s_%s);\n", pinstr->id, approxid, pinstr->name, acadp_approx_get_name(papprox));
      }
      if (acadp_approx_get_regbank_read(papprox, pinstr->id) != NULL) {
        fprintf(fp, "\t\t\tDM_rbnrd[%d].activateModel(%d, &Wrappers::Data::%s_%s_rbnrd);\n", pinstr->id, approxid, pinstr->name, acadp_approx_get_name(papprox));
      }
      if (acadp_approx_get_regbank_write(papprox, pinstr->id) != NULL) {
        fprintf(fp, "\t\t\tDM_rbnwr[%d].activateModel(%d, &Wrappers::Data::%s_%s_rbnwr);\n", pinstr->id, approxid, pinstr->name, acadp_approx_get_name(papprox));
      }
      if (acadp_approx_get_mem_read(papprox, pinstr->id) != NULL) {
        fprintf(fp, "\t\t\tDM_memrd[%d].activateModel(%d, &Wrappers::Data::%s_%s_memrd);\n", pinstr->id, approxid, pinstr->name, acadp_approx_get_name(papprox));
      }
      if (acadp_approx_get_mem_write(papprox, pinstr->id) != NULL) {
        fprintf(fp, "\t\t\tDM_memwr[%d].activateModel(%d, &Wrappers::Data::%s_%s_memwr);\n", pinstr->id, approxid, pinstr->name, acadp_approx_get_name(papprox));
      }
      if (acadp_approx_get_reg_read(papprox, pinstr->id) != NULL) {
        fprintf(fp, "\t\t\tDM_regrd[%d].activateModel(%d, &Wrappers::Data::%s_%s_regrd);\n", pinstr->id, approxid, pinstr->name, acadp_approx_get_name(papprox));
      }
      if (acadp_approx_get_reg_write(papprox, pinstr->id) != NULL) {
        fprintf(fp, "\t\t\tDM_regwr[%d].activateModel(%d, &Wrappers::Data::%s_%s_regwr);\n", pinstr->id, approxid, pinstr->name, acadp_approx_get_name(papprox));
      }
      if (acadp_approx_get_energy(papprox, pinstr->id) != NULL) {
        fprintf(fp, "\t\t\tEM[%d].activateModel(%d, &Wrappers::Energy::%s_%s);\n", pinstr->id, approxid, pinstr->name, acadp_approx_get_name(papprox));
      }
      op = acadp_approx_get_op(papprox, pinstr->id);
      if (op != NULL) {
        print_op(fp, op, pinstr->id, approxid, "\t\t\t");
      }
    }
    fprintf(fp, "\t\t\tbreak; // %s\n\n", acadp_approx_get_name(papprox));
    approxid++;
  }
  fprintf(fp, "\t}\n\n");
  fprintf(fp, "\tthis->active_approx |= static_cast<uint64_t>(0x1) << approx_id;\n");
  fprintf(fp, "}\n\n");

  fprintf(fp, "void AxPIKE::Control::initModels() {\n");
  //fprintf(fp, "\tDM_rbnrd[0].activateModel(0xff, NULL);\n");
  //fprintf(fp, "\tDM_rbnwr[0].activateModel(0xff, NULL);\n");
  //fprintf(fp, "\tDM_memrd[0].activateModel(0xff, NULL);\n");
  //fprintf(fp, "\tDM_memwr[0].activateModel(0xff, NULL);\n");
  //fprintf(fp, "\tDM_regrd[0].activateModel(0xff, NULL);\n");
  //fprintf(fp, "\tDM_regwr[0].activateModel(0xff, NULL);\n");
  //fprintf(fp, "\tEM[0].activateModel(0xff, &Wrappers::Energy::__default__);\n");
  print_op(fp, acadp_get_default_op(), 0, 0xff, "\t");

  pinstr = NULL;
  while(acadp_get_instruction(&pinstr)) {
    //has_approx = false;
    //papprox = NULL;
    //while (acadp_get_approx(&papprox)) {
    //  if (acadp_approx_get_pre_behavior(papprox, pinstr->id) != NULL || acadp_approx_get_post_behavior(papprox, pinstr->id) != NULL || acadp_approx_get_alt_behavior(papprox, pinstr->id) != NULL) {
    //    has_approx = true;
    //    break;
    //  }
    //}
    //if (has_approx) {
    //  fprintf(fp, "\tIM[%d].activateModel(0xff, &Wrappers::Instruction::%s);\n", pinstr->id, pinstr->name);
    //}
    //fprintf(fp, "\tDM_rbnrd[%d].activateModel(0xff, NULL);\n", pinstr->id);
    //fprintf(fp, "\tDM_rbnwr[%d].activateModel(0xff, NULL);\n", pinstr->id);
    //fprintf(fp, "\tDM_memrd[%d].activateModel(0xff, NULL);\n", pinstr->id);
    //fprintf(fp, "\tDM_memwr[%d].activateModel(0xff, NULL);\n", pinstr->id);
    //fprintf(fp, "\tDM_regrd[%d].activateModel(0xff, NULL);\n", pinstr->id);
    //fprintf(fp, "\tDM_regwr[%d].activateModel(0xff, NULL);\n", pinstr->id);
    

    pgroup = NULL;
    //genergy = NULL;
    op = NULL;
    while (acadp_get_group(&pgroup)) {
      if (acadp_group_contains_instruction(pinstr->id, pgroup)) {
        //if (genergy == NULL && acadp_group_get_energy(pgroup) != NULL) {
        //  genergy = acadp_group_get_name(pgroup);
        //}
        if (op == NULL) {
          op = acadp_group_get_op(pgroup);
        }
        if (genergy != NULL && op != NULL) {
          break;
        }
      }
    }
    //if (genergy != NULL) {
    //  fprintf(fp, "\tEM[%d].activateModel(0xff, &Wrappers::Energy::%s);\n", pinstr->id, genergy);
    //}
    //else {
    //  fprintf(fp, "\tEM[%d].activateModel(0xff, &Wrappers::Energy::__default__);\n", pinstr->id);
    //}
    if (op != NULL) {
      print_op(fp, op, pinstr->id, 0xff, "\t");
    }
    else {
      print_op(fp, acadp_get_default_op(), pinstr->id, 0xff, "\t");
    }
  }

  approxid = 0;
  papprox = NULL;
  while (acadp_get_approx(&papprox)) {
    if (acadp_approx_get_default_st(papprox) == ST_ON) {
      fprintf(fp, "\tthis->activateApprox(%d); // %s\n", approxid, acadp_approx_get_name(papprox));
    }
    approxid++;
  }
  fprintf(fp, "\tthis->stats.setStats();\n\n");

  approxid = 0;
  papprox = NULL;
  while (acadp_get_approx(&papprox)) {
    fprintf(fp, "\tthis->approxes[\"%s\"] = %d;\n", acadp_approx_get_name(papprox), approxid);
    approxid++;
  }

  fprintf(fp, "} // initModels()\n\n");

  approxid = 0;
  papprox = NULL;
  fprintf(fp, "void AxPIKE::Stats::initStats() {\n");
  while (acadp_get_approx(&papprox)) {
    fprintf(fp, "\tAxPIKE::Stats::approxes[%d] = \"%s\";\n", approxid, acadp_approx_get_name(papprox));
    approxid++;
  }
  fprintf(fp, "}\n\n");

  fclose(fp);
}

void create_wrappers_header() {
  FILE *fp = fopen("axpike_wrappers.h", "w");

  AcadpApprox* papprox = NULL;
  AcadpGroup* pgroup = NULL;
  AcadpInstruction* pinstr = NULL;
  bool has_approx = false;

  fprintf(fp, "#ifndef _AXPIKE_WRAPPERS_H_\n");
  fprintf(fp, "#define _AXPIKE_WRAPPERS_H_\n\n");

  fprintf(fp, "#include \"insn_template.h\"\n\n");

  fprintf(fp, "namespace AxPIKE {\n");
  fprintf(fp, "\tnamespace Wrappers {\n");
  fprintf(fp, "\t\tnamespace Instruction {\n");

  pinstr = NULL;
  while (acadp_get_instruction(&pinstr)) {
    papprox = NULL;
    has_approx = false;

    while (acadp_get_approx(&papprox)) {
      if (acadp_approx_get_pre_behavior(papprox, pinstr->id) != NULL || acadp_approx_get_post_behavior(papprox, pinstr->id) != NULL || acadp_approx_get_alt_behavior(papprox, pinstr->id) != NULL) {
        has_approx = true;
        fprintf(fp, "\t\t\tvoid %s_%s(processor_t* p, insn_t insn, reg_t pc, int xlen, reg_t npc, source_t* source, void* data);\n", pinstr->name, acadp_approx_get_name(papprox));
      }
    }
    //if (has_approx) {
    //  fprintf(fp, "\t\t\tvoid %s(processor_t* p, insn_t insn, reg_t pc, int xlen, reg_t npc, source_t* source, void* data);\n\n", pinstr->name);
    //}
  }

  fprintf(fp, "\t\t} // namespace AxPIKE::Wrappers::Instruction\n");

  fprintf(fp, "\t\tnamespace Data {\n");
  pinstr = NULL;
  while (acadp_get_instruction(&pinstr)) {
    papprox = NULL;
    while (acadp_get_approx(&papprox)) {
      has_approx = false;
      if (acadp_approx_get_regbank_read(papprox, pinstr->id) != NULL) {
        fprintf(fp, "\t\t\tvoid %s_%s_rbnrd(processor_t* p, insn_t insn, reg_t pc, int xlen, reg_t npc, source_t* source, void* data);\n", pinstr->name, acadp_approx_get_name(papprox));
        has_approx = true;
      }
      if (acadp_approx_get_regbank_write(papprox, pinstr->id) != NULL) {
        fprintf(fp, "\t\t\tvoid %s_%s_rbnwr(processor_t* p, insn_t insn, reg_t pc, int xlen, reg_t npc, source_t* source, void* data);\n", pinstr->name, acadp_approx_get_name(papprox));
        has_approx = true;
      }
      if (acadp_approx_get_mem_read(papprox, pinstr->id) != NULL) {
        fprintf(fp, "\t\t\tvoid %s_%s_memrd(processor_t* p, insn_t insn, reg_t pc, int xlen, reg_t npc, source_t* source, void* data);\n", pinstr->name, acadp_approx_get_name(papprox));
        has_approx = true;
      }
      if (acadp_approx_get_mem_write(papprox, pinstr->id) != NULL) {
        fprintf(fp, "\t\t\tvoid %s_%s_memwr(processor_t* p, insn_t insn, reg_t pc, int xlen, reg_t npc, source_t* source, void* data);\n", pinstr->name, acadp_approx_get_name(papprox));
        has_approx = true;
      }
      if (acadp_approx_get_reg_read(papprox, pinstr->id) != NULL) {
        fprintf(fp, "\t\t\tvoid %s_%s_regrd(processor_t* p, insn_t insn, reg_t pc, int xlen, reg_t npc, source_t* source, void* data);\n", pinstr->name, acadp_approx_get_name(papprox));
        has_approx = true;
      }
      if (acadp_approx_get_reg_write(papprox, pinstr->id) != NULL) {
        fprintf(fp, "\t\t\tvoid %s_%s_regwr(processor_t* p, insn_t insn, reg_t pc, int xlen, reg_t npc, source_t* source, void* data);\n", pinstr->name, acadp_approx_get_name(papprox));
        has_approx = true;
      }
    }
    if (has_approx) {
      fprintf(fp, "\n");
    }
  }
  fprintf(fp, "\t\t}// namespace AxPIKE::Wrappers::Data\n\n");

  fprintf(fp, "\t\tnamespace Energy {\n");
  fprintf(fp, "\t\t\tvoid __default__(processor_t* p, insn_t insn, reg_t pc, int xlen, reg_t npc, source_t* source, void* data);\n\n");
  pinstr = NULL;
  while (acadp_get_instruction(&pinstr)) {
    papprox = NULL;
    while (acadp_get_approx(&papprox)) {
      if (acadp_approx_get_energy(papprox, pinstr->id) != NULL) {
        fprintf(fp, "\t\t\tvoid %s_%s(processor_t* p, insn_t insn, reg_t pc, int xlen, reg_t npc, source_t* source, void* data);\n", pinstr->name, acadp_approx_get_name(papprox));
      }
    }
  }
  while (acadp_get_group(&pgroup)) {
    if (acadp_group_get_energy(pgroup) != NULL) {
      fprintf(fp, "\t\t\tvoid %s(processor_t* p, insn_t insn, reg_t pc, int xlen, reg_t npc, source_t* source, void* data);\n", acadp_group_get_name(pgroup));
    }
  }
  fprintf(fp, "\t\t}// namespace AxPIKE::Wrappers::Energy\n\n");

  fprintf(fp, "\t} // namespace AxPIKE::Wrappers\n");
  fprintf(fp, "} // namespace AxPIKE\n");

  fprintf(fp, "#endif //#ifndef _AXPIKE_WRAPPERS_H_\n");

  fclose(fp);
}

void create_wrappers_impl() {
  FILE *fp = fopen("axpike_wrappers.cc","w");

  AcadpApprox* papprox = NULL;
  AcadpInstruction* pinstr = NULL;
  AcadpGroup* pgroup = NULL;
  bool has_approx;

  char *energy;
  char *probability;
  char *pre_bhv;
  char *pos_bhv;
  char *alt_bhv;
  char *rbnrd;
  char *rbnwr;
  char *memrd;
  char *memwr;
  char *regrd;
  char *regwr;

  fprintf(fp, "#ifndef _AXPIKE_WRAPPERS_CC_\n");
  fprintf(fp, "#define _AXPIKE_WRAPPERS_CC_\n\n");
  fprintf(fp, "#include \"axpike_wrappers.h\"\n\n");
  fprintf(fp, "typedef AxPIKE::Word<reg_t>& word;\n");
  fprintf(fp, "typedef AxPIKE::Word<sreg_t>& sword;\n");
  fprintf(fp, "typedef AxPIKE::Word<freg_t>& fword;\n\n");
  fprintf(fp, "namespace AxPIKE {\n");
  fprintf(fp, "\tnamespace Models {\n");
  fprintf(fp, "\t\tnamespace Instruction {\n");
  acadp_write_models_header(fp, MODEL_IM, "\t\t\t");
  fprintf(fp, "\t\t}\n");
  fprintf(fp, "\t\tnamespace Data {\n");
  acadp_write_models_header(fp, MODEL_DM, "\t\t\t");
  fprintf(fp, "\t\t}\n");
  fprintf(fp, "\t\tnamespace Probability {\n");
  acadp_write_models_header(fp, MODEL_PM, "\t\t\t");
  fprintf(fp, "\t\t}\n");
  fprintf(fp, "\t\tnamespace Energy {\n");
  acadp_write_models_header(fp, MODEL_EM, "\t\t\t");
  fprintf(fp, "\t\t}\n");
  fprintf(fp, "\t}\n");
  fprintf(fp, "}\n\n");
  fprintf(fp, "#define IM inline __attribute__((always_inline)) void AxPIKE::Models::Instruction::\n");
  fprintf(fp, "#define DM inline __attribute__((always_inline)) void AxPIKE::Models::Data::\n");
  fprintf(fp, "#define PM inline __attribute__((always_inline)) bool AxPIKE::Models::Probability::\n");
  fprintf(fp, "#define EM inline __attribute__((always_inline)) double AxPIKE::Models::Energy::\n");
  fprintf(fp, "#define OP p->ax_control.OP\n");
  fprintf(fp, "#include \"axpike_models.cc\"\n");
  fprintf(fp, "#undef IM\n");
  fprintf(fp, "#undef DM\n");
  fprintf(fp, "#undef PM\n");
  fprintf(fp, "#undef EM\n");
  fprintf(fp, "#undef OP\n\n");

  while (acadp_get_instruction(&pinstr)) {
    papprox = NULL;
    has_approx = false;
    while (acadp_get_approx(&papprox)) {
      probability = acadp_approx_get_probability(papprox, pinstr->id);
      pre_bhv = acadp_approx_get_pre_behavior(papprox, pinstr->id);
      pos_bhv = acadp_approx_get_post_behavior(papprox, pinstr->id);
      alt_bhv = acadp_approx_get_alt_behavior(papprox, pinstr->id);

      if (pre_bhv != NULL || pos_bhv != NULL || alt_bhv != NULL) {
        fprintf(fp, "void AxPIKE::Wrappers::Instruction::%s_%s(processor_t* p, insn_t insn, reg_t pc, int xlen, reg_t npc, source_t* source, void* data){\n", pinstr->name, acadp_approx_get_name(papprox));
        if (probability != NULL) {
          fprintf(fp, "\tif (AxPIKE::Models::Probability::%s) {\n", probability);
        }
        if (pre_bhv != NULL) {
          fprintf(fp, "\t\tAxPIKE::Models::Instruction::%s;\n", pre_bhv);
        }
        if (alt_bhv != NULL) {
          fprintf(fp, "\t\tAxPIKE::Models::Instruction::%s;\n", alt_bhv);
        }
        else {
          fprintf(fp, "\t\t#include \"insns/%s.h\"\n", pinstr->name);
        }
        if (pos_bhv != NULL) {
          fprintf(fp, "\t\tAxPIKE::Models::Instruction::%s;\n", pos_bhv);
        }
        if (probability != NULL) {
          fprintf(fp, "\t}\n");
          fprintf(fp, "\telse {\n");
          fprintf(fp, "\t\t#include \"insns/%s.h\"\n", pinstr->name);
          fprintf(fp, "\t}\n");
        }
        fprintf(fp, "}\n\n");
      }

    }
  }

  pinstr = NULL;
  while (acadp_get_instruction(&pinstr)) {
    papprox = NULL;
    while (acadp_get_approx(&papprox)) {
      probability = acadp_approx_get_probability(papprox, pinstr->id);
      rbnrd = acadp_approx_get_regbank_read(papprox, pinstr->id);
      rbnwr = acadp_approx_get_regbank_write(papprox, pinstr->id);
      memrd = acadp_approx_get_mem_read(papprox, pinstr->id);
      memwr = acadp_approx_get_mem_write(papprox, pinstr->id);
      regrd = acadp_approx_get_reg_read(papprox, pinstr->id);
      regwr = acadp_approx_get_reg_write(papprox, pinstr->id);
      if (rbnrd != NULL) {
        fprintf(fp, "void AxPIKE::Wrappers::Data::%s_%s_rbnrd(processor_t* p, insn_t insn, reg_t pc, int xlen, reg_t npc, source_t* source, void* data){\n", pinstr->name, acadp_approx_get_name(papprox));
        if (probability != NULL) {
          fprintf(fp, "\tif (AxPIKE::Models::Probability::%s) {\n", probability);
        }
        fprintf(fp, "\t\tAxPIKE::Models::Data::%s;\n", rbnrd);
        if (probability != NULL) {
          fprintf(fp, "\t}\n");
        }
        fprintf(fp, "}\n\n");
      }
      if (rbnwr != NULL) {
        fprintf(fp, "void AxPIKE::Wrappers::Data::%s_%s_rbnwr(processor_t* p, insn_t insn, reg_t pc, int xlen, reg_t npc, source_t* source, void* data){\n", pinstr->name, acadp_approx_get_name(papprox));
        if (probability != NULL) {
          fprintf(fp, "\tif (AxPIKE::Models::Probability::%s) {\n", probability);
        }
        fprintf(fp, "\t\tAxPIKE::Models::Data::%s;\n", rbnwr);
        if (probability != NULL) {
          fprintf(fp, "\t}\n");
        }
        fprintf(fp, "}\n\n");
      }
      if (memrd != NULL) {
        fprintf(fp, "void AxPIKE::Wrappers::Data::%s_%s_memrd(processor_t* p, insn_t insn, reg_t pc, int xlen, reg_t npc, source_t* source, void* data){\n", pinstr->name, acadp_approx_get_name(papprox));
        if (probability != NULL) {
          fprintf(fp, "\tif (AxPIKE::Models::Probability::%s) {\n", probability);
        }
        fprintf(fp, "\t\tAxPIKE::Models::Data::%s;\n", memrd);
        if (probability != NULL) {
          fprintf(fp, "\t}\n");
        }
        fprintf(fp, "}\n\n");
      }
      if (memwr != NULL) {
        fprintf(fp, "void AxPIKE::Wrappers::Data::%s_%s_memwr(processor_t* p, insn_t insn, reg_t pc, int xlen, reg_t npc, source_t* source, void* data){\n", pinstr->name, acadp_approx_get_name(papprox));
        if (probability != NULL) {
          fprintf(fp, "\tif (AxPIKE::Models::Probability::%s) {\n", probability);
        }
        fprintf(fp, "\t\tAxPIKE::Models::Data::%s;\n", memwr);
        if (probability != NULL) {
          fprintf(fp, "\t}\n");
        }
        fprintf(fp, "}\n\n");
      }
      if (regrd != NULL) {
        fprintf(fp, "void AxPIKE::Wrappers::Data::%s_%s_regrd(processor_t* p, insn_t insn, reg_t pc, int xlen, reg_t npc, source_t* source, void* data){\n", pinstr->name, acadp_approx_get_name(papprox));
        if (probability != NULL) {
          fprintf(fp, "\tif (AxPIKE::Models::Probability::%s) {\n", probability);
        }
        fprintf(fp, "\t\tAxPIKE::Models::Data::%s;\n", regrd);
        if (probability != NULL) {
          fprintf(fp, "\t}\n");
        }
        fprintf(fp, "}\n\n");
      }
      if (regwr != NULL) {
        fprintf(fp, "void AxPIKE::Wrappers::Data::%s_%s_regwr(processor_t* p, insn_t insn, reg_t pc, int xlen, reg_t npc, source_t* source, void* data){\n", pinstr->name, acadp_approx_get_name(papprox));
        if (probability != NULL) {
          fprintf(fp, "\tif (AxPIKE::Models::Probability::%s) {\n", probability);
        }
        fprintf(fp, "\t\tAxPIKE::Models::Data::%s;\n", regwr);
        if (probability != NULL) {
          fprintf(fp, "\t}\n");
        }
        fprintf(fp, "}\n\n");
      }
    }
  }

  energy = acadp_get_default_energy();
  fprintf(fp, "void AxPIKE::Wrappers::Energy::__default__(processor_t* p, insn_t insn, reg_t pc, int xlen, reg_t npc, source_t* source, void* data){\n");
  if (energy != NULL) {
    fprintf(fp, "\tp->ax_control.stats.insnDispatch(AxPIKE::Models::Energy::%s);\n", energy);
  }
  else {
    fprintf(fp, "\tp->ax_control.stats.insnDispatch(0.0);\n");
  }
  fprintf(fp, "}\n");

  pinstr = NULL;
  while (acadp_get_instruction(&pinstr)) {
    has_approx = false;
    papprox = NULL;
    while (acadp_get_approx(&papprox)) {
      energy = acadp_approx_get_energy(papprox, pinstr->id);
      if (energy != NULL) {
        has_approx = true;
        fprintf(fp, "void AxPIKE::Wrappers::Energy::%s_%s(processor_t* p, insn_t insn, reg_t pc, int xlen, reg_t npc, source_t* source, void* data){\n", pinstr->name, acadp_approx_get_name(papprox));
        fprintf(fp, "\tp->ax_control.stats.insnDispatch(AxPIKE::Models::Energy::%s);\n", energy);
        fprintf(fp, "}\n");
      }
    }
  }

  while(acadp_get_group(&pgroup)) {
    energy = acadp_group_get_energy(pgroup);
    if (energy != NULL) {
      fprintf(fp, "void AxPIKE::Wrappers::Energy::%s(processor_t* p, insn_t insn, reg_t pc, int xlen, reg_t npc, source_t* source, void* data){\n", acadp_group_get_name(pgroup));
      fprintf(fp, "\tp->ax_control.stats.insnDispatch(AxPIKE::Models::Energy::%s);\n", energy);
      fprintf(fp, "}\n");
    }
  }

  fprintf(fp, "\n#endif\n");

  fclose(fp);
}

void create_instruction_list() {
  FILE *fp = fopen("insn_approx.list","w");

  AcadpApprox* papprox = NULL;
  AcadpInstruction* pinstr = NULL;

  while (acadp_get_instruction(&pinstr)) {
    papprox = NULL;
    while (acadp_get_approx(&papprox)) {
      if (acadp_approx_get_pre_behavior(papprox, pinstr->id) != NULL || acadp_approx_get_post_behavior(papprox, pinstr->id) != NULL || acadp_approx_get_alt_behavior(papprox, pinstr->id) != NULL) {
        fprintf(fp, "%s\n", pinstr->name);
        break;
      }
    }
  }

  fclose(fp);
}

void create_iface_file() {
  FILE *fp = fopen("axpike_iface.h","w");
  AcadpApprox* papprox = NULL;
  int approxid = 0;

  fprintf(fp, "#ifndef _AXPIKE_IFACE_H_\n");
  fprintf(fp, "#define _AXPIKE_IFACE_H_\n\n");

  fprintf(fp, "#ifndef AXPIKE_SYSNUM\n");
  fprintf(fp, "#define AXPIKE_SYSNUM 380\n");
  fprintf(fp, "#endif\n\n");

  while (acadp_get_approx(&papprox)) {
    fprintf(fp, "#define AXPIKE_%s 0x%02x\n", acadp_approx_get_name(papprox), approxid++);
  }
  fprintf(fp, "\n\n");

  fprintf(fp, "#ifndef _AXPIKE_SUPPRESS_\n");
  fprintf(fp, "#define axpike_activate(x) asm volatile (\"li a7,%%0\\nli a0,%%1\\necall\" : : \"i\"(AXPIKE_SYSNUM), \"i\"((x&0x3f)|0x40) : \"a0\",\"a7\")\n");
  fprintf(fp, "#define axpike_bactivate(x) asm volatile (\"li a7,%%0\\nli a0,%%1\\necall\" : : \"i\"(AXPIKE_SYSNUM), \"i\"((x&0x3f)|0x140) : \"a0\",\"a7\")\n");
  fprintf(fp, "#define axpike_deactivate(x) asm volatile (\"li a7,%%0\\nli a0,%%1\\necall\" : : \"i\"(AXPIKE_SYSNUM), \"i\"(x&0x3f) : \"a0\",\"a7\")\n");
  fprintf(fp, "#define axpike_bdeactivate(x) asm volatile (\"li a7,%%0\\nli a0,%%1\\necall\" : : \"i\"(AXPIKE_SYSNUM), \"i\"((x&0x3f)|0x100) : \"a0\",\"a7\")\n");
  fprintf(fp, "#define axpike_clear() asm volatile (\"li a7,%%0\\nli a0,0xff\\necall\" : : \"i\"(AXPIKE_SYSNUM) : \"a0\",\"a7\")\n");
  fprintf(fp, "#define axpike_bclear() asm volatile (\"li a7,%%0\\nli a0,0x1ff\\necall\" : : \"i\"(AXPIKE_SYSNUM) : \"a0\",\"a7\")\n\n");
  fprintf(fp, "#else\n");
  fprintf(fp, "#define axpike_activate(x)\n");
  fprintf(fp, "#define axpike_bactivate(x)\n");
  fprintf(fp, "#define axpike_deactivate(x)\n");
  fprintf(fp, "#define axpike_bdeactivate(x)\n");
  fprintf(fp, "#define axpike_clear()\n");
  fprintf(fp, "#define axpike_bclear()\n");
  fprintf(fp, "#endif\n\n");
  fprintf(fp, "#define axpike_newsection() asm volatile (\"li a7,%%0\\nli a0,0x1fe\\necall\" : : \"i\"(AXPIKE_SYSNUM) : \"a0\",\"a7\")\n");
  fprintf(fp, "#define axpike_bnewsection() axpike_newsection()\n\n");

  fprintf(fp, "#endif // _AXPIKE_IFACE_H_\n");

  fclose(fp);
}

extern int adfparse();
extern FILE* adfin;
extern int adf_line_num;

void acadp_init() {
  acadp_init_models();
  acadp_init_approx();
  acadp_init_group();
  acadp_init_op();
}

int acadp_load(char* file) {
  adfin = fopen(file, "r");
  if (adfin == NULL) {
    return 0;
  }
  else {
    return 1;
  }
}

void acadp_destroy() {
  if (adfin != NULL) {
    fclose(adfin);
  }
  adfin = NULL;
  acadp_destroy_models();
  acadp_destroy_approx();
  acadp_destroy_group();
  acadp_destroy_op();
}

int acadp_run() {
  adf_line_num = 1;
  return adfparse();
}
