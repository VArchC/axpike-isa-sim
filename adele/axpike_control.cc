#include "axpike_control.h"
#include <iostream>
#include "insn_count.h"

AxPIKE::Control::Control(processor_t* _p, std::vector<std::string> adele_activate, std::vector<std::string> adele_deactivate) : 
  active_approx(0),
  cur_insn_id(0),
  stats(_p, cur_insn_id, active_approx),
  p(_p),
  OP(cur_insn_id) {
    this->initModels();


    for (std::string& a : adele_activate) {
      if (approxes.count(a) == 1) {
        activateApprox(approxes[a]);
      }
    }
    for (std::string& a : adele_deactivate) {
      if (approxes.count(a) == 1) {
        for (int i=0; i < INSN_COUNT; i++) {
          IM[i].deactivateModel(approxes[a]);
          DM_regrd[i].deactivateModel(approxes[a]);
          DM_regwr[i].deactivateModel(approxes[a]);
          DM_rbnrd[i].deactivateModel(approxes[a]);
          DM_rbnwr[i].deactivateModel(approxes[a]);
          DM_memrd[i].deactivateModel(approxes[a]);
          DM_memwr[i].deactivateModel(approxes[a]);
          EM[i].deactivateModel(approxes[a]);
        }
        OP.deactivateOP(approxes[a]);
        this->active_approx &= ~(static_cast<uint64_t>(0x1) << approxes[a]);
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
  if (params.find(cur_insn_id) == params.end()) {
    return std::numeric_limits<double>::quiet_NaN();
  }
  else {
    return params[cur_insn_id].front()[key];
  }
}

void AxPIKE::OPList::activateOP(uint32_t insn, uint8_t approx_id, op_t params) {
  if (this->params[insn].empty()) {
    this->params[insn].emplace_front(approx_id, params);
  }
  else {
    this->params[insn].emplace_front(approx_id, this->params[insn].front(), params);
  }
}

void AxPIKE::OPList::deactivateOP(uint8_t approx_id) {
  OPElement rm(approx_id);
  for (auto& x: this->params) {
    x.second.remove(rm);
  }
}

void AxPIKE::OPList::clear() {
  for (auto& x: this->params) {
    x.second.remove_if([](const OPElement& cmp){return (cmp.approx_id != 0xff);});
  }
}

void AxPIKE::Control::issue(uint8_t cmd) {
  if (cmd < 64) {
    for (int i=0; i < INSN_COUNT; i++) {
      IM[i].deactivateModel(cmd);
      DM_regrd[i].deactivateModel(cmd);
      DM_regwr[i].deactivateModel(cmd);
      DM_rbnrd[i].deactivateModel(cmd);
      DM_rbnwr[i].deactivateModel(cmd);
      DM_memrd[i].deactivateModel(cmd);
      DM_memwr[i].deactivateModel(cmd);
      EM[i].deactivateModel(cmd);
    }
    OP.deactivateOP(cmd);
    this->active_approx &= ~(static_cast<uint64_t>(0x1) << cmd);
    this->stats.setStats();
  }
  else if (cmd < 128) {
    this->activateApprox( cmd & 0x3f );
    this->stats.setStats();
  }
  else if (cmd == 0xff) {
    for (int i=0; i < INSN_COUNT; i++) {
      IM[i].clear();
      DM_regrd[i].clear();
      DM_regwr[i].clear();
      DM_rbnrd[i].clear();
      DM_rbnwr[i].clear();
      DM_memrd[i].clear();
      DM_memwr[i].clear();
      EM[i].clear();
    }
    this->active_approx = 0;
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
