#include "axpike_control.h"
#include "processor.h"
#include <iostream>
#include "insn_count.h"

AxPIKE::Control::Control(processor_t* _p, std::vector<std::string> adele_activate, std::vector<std::string> adele_deactivate) : 
  active_approx{0,0,0,0},
  cur_insn_id(0),
  prv(_p->state.prv),
  stats(_p, cur_insn_id, active_approx),
  log(_p),
  counters(_p),
  p(_p),
  OP(cur_insn_id, _p->state.prv) {
    this->initModels();


    for (std::string& a : adele_activate) {
      if (approxes.count(a) == 1) {
        activateApprox(approxes[a], 0);
        activateApprox(approxes[a], 1);
        activateApprox(approxes[a], 2);
        activateApprox(approxes[a], 3);
      }
    }
    for (std::string& a : adele_deactivate) {
      if (approxes.count(a) == 1) {
        for (int i=0; i < (INSN_COUNT << 2); i++) {
          IM[i].deactivateModel(approxes[a]);
          DM_regrd[i].deactivateModel(approxes[a]);
          DM_regwr[i].deactivateModel(approxes[a]);
          DM_rbnrd[i].deactivateModel(approxes[a]);
          DM_rbnwr[i].deactivateModel(approxes[a]);
          DM_memrd[i].deactivateModel(approxes[a]);
          DM_memwr[i].deactivateModel(approxes[a]);
          EM[i].deactivateModel(approxes[a]);
        }
        OP.deactivateOP(approxes[a], 0);
        OP.deactivateOP(approxes[a], 1);
        OP.deactivateOP(approxes[a], 2);
        OP.deactivateOP(approxes[a], 3);
        this->active_approx[0] &= ~(static_cast<uint64_t>(0x1) << approxes[a]);
        this->active_approx[1] &= ~(static_cast<uint64_t>(0x1) << approxes[a]);
        this->active_approx[2] &= ~(static_cast<uint64_t>(0x1) << approxes[a]);
        this->active_approx[3] &= ~(static_cast<uint64_t>(0x1) << approxes[a]);
      }
    }

    this->stats.setStats();
}

void AxPIKE::WrapperList::activateModel(uint8_t approx_id, wrapper_t func) {
  this->wrappers.emplace_front(approx_id, func);
  this->func = (wrappers.front()).func;
}

void AxPIKE::WrapperList::deactivateModel(uint8_t approx_id) {
  this->wrappers.remove(WrapperElement(approx_id));
  if (this->wrappers.empty()) {
    this->func = NULL;
  }
  else {
    this->func = (wrappers.front()).func;
  }
}

void AxPIKE::WrapperList::clear() {
  this->wrappers.clear();
  this->func = NULL;
}

AxPIKE::OPElement::OPElement(uint8_t _approx_id, const OPElement& previous, op_t _params) : approx_id(_approx_id) {
  for (auto& x: previous.params) {
    params[x.first] = x.second;
  }
  for(auto& x: _params) {
    params[x.first] = x.second;
  }
}

double AxPIKE::OPElement::operator[] (const std::string& key) {
  if (params.find(key) == params.end()) {
    return std::numeric_limits<double>::quiet_NaN();
  }
  else {
    return params[key];
  }
}

double AxPIKE::OPList::operator[] (const std::string& key) {
  if (params.find((cur_insn_id << 2) | prv) == params.end()) {
    return std::numeric_limits<double>::quiet_NaN();
  }
  else {
    return params[(cur_insn_id << 2) | prv].front()[key];
  }
}

void AxPIKE::OPList::activateOP(uint32_t insn, uint64_t prv, uint8_t approx_id, op_t params) {
  if (this->params[(insn << 2) | prv].empty()) {
    this->params[(insn << 2) | prv].emplace_front(approx_id, params);
  }
  else {
    this->params[(insn << 2) | prv].emplace_front(approx_id, this->params[(insn << 2) | prv].front(), params);
  }
}

void AxPIKE::OPList::deactivateOP(uint8_t approx_id, uint64_t prv) {
  OPElement rm(approx_id);
  for (auto& x: this->params) {
    if ((x.first & 0x3) == prv) {
      x.second.remove(rm);
    }
  }
}

void AxPIKE::OPList::clear() {
  for (auto& x: this->params) {
    x.second.remove_if([](const OPElement& cmp){return (cmp.approx_id != 0xff);});
  }
}

void AxPIKE::Control::deactivateApprox(uint8_t approx_id, uint64_t prv) {
  for (int i=prv; i < (INSN_COUNT << 2); i+=4) {
    IM[i].deactivateModel(approx_id);
    DM_regrd[i].deactivateModel(approx_id);
    DM_regwr[i].deactivateModel(approx_id);
    DM_rbnrd[i].deactivateModel(approx_id);
    DM_rbnwr[i].deactivateModel(approx_id);
    DM_memrd[i].deactivateModel(approx_id);
    DM_memwr[i].deactivateModel(approx_id);
    EM[i].deactivateModel(approx_id);
  }
}

void AxPIKE::Control::issue(uint8_t cmd) {
  if (cmd < 64) {
    this->deactivateApprox(cmd, 0);
    this->deactivateApprox(cmd, 1);
    this->deactivateApprox(cmd, 2);
    this->deactivateApprox(cmd, 3);
    OP.deactivateOP(cmd, 0);
    OP.deactivateOP(cmd, 1);
    OP.deactivateOP(cmd, 2);
    OP.deactivateOP(cmd, 3);
    this->active_approx[0] &= ~(static_cast<uint64_t>(0x1) << cmd);
    this->active_approx[1] &= ~(static_cast<uint64_t>(0x1) << cmd);
    this->active_approx[2] &= ~(static_cast<uint64_t>(0x1) << cmd);
    this->active_approx[3] &= ~(static_cast<uint64_t>(0x1) << cmd);
    this->stats.setStats();
  }
  else if (cmd < 128) {
    this->activateApprox( cmd & 0x3f , 0);
    this->activateApprox( cmd & 0x3f , 1);
    this->activateApprox( cmd & 0x3f , 2);
    this->activateApprox( cmd & 0x3f , 3);
    this->stats.setStats();
  }
  else if (cmd == 0xff) {
    for (int i=0; i < (INSN_COUNT << 2); i++) {
      IM[i].clear();
      DM_regrd[i].clear();
      DM_regwr[i].clear();
      DM_rbnrd[i].clear();
      DM_rbnwr[i].clear();
      DM_memrd[i].clear();
      DM_memwr[i].clear();
      EM[i].clear();
    }
    this->active_approx[0] = 0;
    this->active_approx[1] = 0;
    this->active_approx[2] = 0;
    this->active_approx[3] = 0;
    this->initModels();
    this->stats.setStats();
  }
  else if (cmd == 0xfe) {
    this->stats.section++;
    this->stats.setStats();
  }
  else if (cmd == 0xfd) {
    this->stats.printCounters();
  }
  else if (cmd == 0xfc) {
    this->stats.clearCounters();
  }
} // issue()
