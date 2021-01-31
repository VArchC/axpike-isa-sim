#include "acadp.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

AcadpModelInst* default_energy = NULL;
AcadpOP* default_op = NULL;

int acadp_set_default_energy(AcadpModelInst* model) {
  AcadpModel* m;
  m = acadp_find_model(model->name, MODEL_EM, model->n_params + 1);
  if(m == NULL) {
    acadp_delete_modelinst(model);
    return 0;
  }
  acadp_add_param_to_modelinst(model, "p");

  default_energy = model;
  return 1;
}

int acadp_set_default_op(char* op_name) {
  AcadpOP* op;
  op = acadp_find_op(op_name);
  if(op == NULL) {
    return 0;
  }

  default_op = op;
  return 1;
}

char* acadp_get_default_energy() {
  if (default_energy != NULL) {
    return acadp_get_modelinst(default_energy);
  }
  else {
    return NULL;
  }
}

AcadpOP* acadp_get_default_op() {
  return default_op;
}
