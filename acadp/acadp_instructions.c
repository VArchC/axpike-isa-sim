#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "acadp.h"

AcadpInstruction* first_instruction;
AcadpInstruction* last_instruction;

int acadp_load_instructions(char *file) {
  FILE *fp;
  char* line = NULL;
  size_t len = 0;
  ssize_t read;
  int instr_id = 0;

  fp = fopen(file, "r");

  if (fp == NULL) {
  }

  while (((read = getline(&line, &len, fp)) != -1) && strlen(line) > 11) {
    AcadpInstruction* instr = malloc(sizeof(AcadpInstruction));
    instr->id = instr_id++;

    line[strlen(line)-2] = '\0';
    instr->name = malloc((len-13)*sizeof(char));
    strcpy(instr->name, &line[12]);

    if (last_instruction != NULL) {
      last_instruction->next = instr;
    }
    else {
      first_instruction = instr;
    }
    last_instruction = instr;
  }

  return --instr_id;
}

void acadp_destroy_instructions() {
  AcadpInstruction* i1 = NULL, *i2 = NULL;
  
  i1 = first_instruction;
  while (i1 != NULL) {
    free(i1->name);
    i2 = i1->next;
    free(i1);
    i1 = i2;
  }

  first_instruction = NULL;
  last_instruction = NULL;
}

AcadpInstruction* acadp_find_instruction(char *name) {
  AcadpInstruction* instr = first_instruction;

  while(instr != NULL && strcmp(name, instr->name) != 0) {
    instr = instr->next;
  }

  return instr;
}

int acadp_get_instruction(AcadpInstruction** instr) {
  if (*instr == NULL) {
    *instr = first_instruction;
  }
  else {
    *instr = (*instr)->next;
  }

  if (*instr == NULL) {
    return 0;
  }
  else {
    return 1;
  }
}
