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
  OP(cur_insn_id, _p->state.prv),
  available_approx{0,0,0,0},
  ac_behavior(~0x0ULL),
  dc_behavior(~0x0ULL) {
    this->initModels();
    this->available_approx[PRV_M] = (0x1ULL << this->approxes.size()) - 1;


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
    OP.clear();
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

bool AxPIKE::Control::identify_csr(int which) {
  return (
          which == CSR_URKAV ||
          which == CSR_URKST ||
          which == CSR_SRKAV ||
          which == CSR_SRKST ||
          which == CSR_MRKAV ||
          which == CSR_MRKST ||
          which == CSR_MRKACBHV ||
          which == CSR_MRKDCBHV ||
          which == CSR_MRKGROUP ||
          which == CSR_MRKADDR
      );
}

void AxPIKE::Control::setApprox_csr(uint64_t req_approx, uint64_t req_prv) {
  uint8_t approx_id = 0;
  uint64_t diff = (req_approx ^ active_approx[req_prv]) & available_approx[prv];

  while (diff) {
    if (diff & 0x1) {
      if (req_approx & 0x1) {
        activateApprox(approx_id, req_prv);
      }
      else {
        deactivateApprox(approx_id, req_prv);
      }
    }

    approx_id++;
    req_approx = req_approx >> 1;
    diff = diff >> 1;
  }
}

void AxPIKE::Control::set_csr(int which, uint64_t val) {
  switch (which) {
    case CSR_URKAV:
      if (prv > PRV_U) {
        available_approx[PRV_U] = (val & available_approx[PRV_M]);
      }
      break;

    case CSR_URKST:
      setApprox_csr(val, PRV_U);
      break;

    case CSR_SRKAV:
      if (prv > PRV_S) {
        available_approx[PRV_S] = (val & available_approx[PRV_M]);
      }
      break;

    case CSR_SRKST:
      setApprox_csr(val, PRV_S);
      break;

    case CSR_MRKST:
      setApprox_csr(val, PRV_M);
      break;

    case CSR_MRKACBHV:
      this->ac_behavior = val;
      break;

    case CSR_MRKDCBHV:
      this->dc_behavior = val;
      break;

    case CSR_MRKGROUP:
      this->issue(0xff); // clear
      break;
  }
}

uint64_t AxPIKE::Control::get_csr(int which) {
  uint64_t r = 0;
  switch (which) {
    case CSR_URKAV:
      r = available_approx[PRV_U];
      break;

    case CSR_URKST:
      r = active_approx[PRV_U];
      if (prv == PRV_U) {
        r &= available_approx[PRV_U];
      }
      break;

    case CSR_SRKAV:
      r = available_approx[PRV_S];
      break;

    case CSR_SRKST:
      r = active_approx[PRV_S];
      if (prv == PRV_S) {
        r &= available_approx[PRV_S];
      }
      break;

    case CSR_MRKAV:
      r = available_approx[PRV_M];
      break;

    case CSR_MRKST:
      r = active_approx[PRV_M];
      break;

    case CSR_MRKACBHV:
      r = ac_behavior;
      break;

    case CSR_MRKDCBHV:
      r = dc_behavior;
      break;

    //case CSR_MRKGROUP:
    //  r = 0;
    //  break;

    //case CSR_MRKADDR:
    //  r = 0;
    //  break;
  }
  return r;
}
