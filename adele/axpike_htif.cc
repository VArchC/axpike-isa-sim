#include <iostream>
#include <functional>
#include "axpike_htif.h"
#include "htif.h"
#include "sim.h"
#include "processor.h"

AxPIKE::HTIF::HTIF() : sim(NULL) {
  register_command(0, std::bind(&AxPIKE::HTIF::handle_control, this, std::placeholders::_1), "control");
}

void AxPIKE::HTIF::initialize(sim_t* _sim) {
  sim = _sim;
  processor_t* p;
  for (uint32_t i = 0; i < sim->nprocs(); i++) {
    p = sim->get_core(i);
    harts[p->get_id()] = p;
  }
}

void AxPIKE::HTIF::handle_control(command_t cmd) {
  if (likely(sim != NULL)) {
    std::unordered_map<uint32_t,processor_t*>::const_iterator it = harts.find(cmd.payload() >> 9);
    if (likely(it != harts.end())) {
      it->second->ax_control.issue(cmd.payload() & 0xFF);
    }
  }
  if (cmd.payload() & 0x100) {
    cmd.respond(1);
  }
}
