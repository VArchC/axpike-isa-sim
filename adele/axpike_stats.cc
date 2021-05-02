#include "axpike_stats.h"
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <processor.h>
#include "insn_count.h"
#include "mmu.h"

std::vector<std::string> AxPIKE::Stats::insns;
std::unordered_map<uint8_t, std::string> AxPIKE::Stats::approxes;

AxPIKE::Stats::~Stats() {
  printCounters();
}

void AxPIKE::Stats::printCounters() {
  std::string fname;
  fname = "AxPIKE_counters_" + std::to_string(::getpid()) + "_hart" + std::to_string(p->get_id()) + "_" + std::to_string(seq) + ".csv";
  printInstrCounter(fname.c_str());
  fname = "AxPIKE_energy_" + std::to_string(::getpid()) + "_hart" + std::to_string(p->get_id()) + "_" + std::to_string(seq++) + ".csv";
  printEnergyCounter(fname.c_str());
  clearCounters();
}

void AxPIKE::Stats::clearCounters() {
  icounter_map.clear();
  ecounter_map.clear();
  setStats();
}

void AxPIKE::Stats::printInstrCounter(const char* fname) {
  std::ofstream fp;
  fp.open(fname, std::ios::out | std::ios::trunc);

  const char *prvs[4] = {"U", "S", "HS", "M"};

  std::cerr << "Writing AxPIKE instruction counters to " << fname << std::endl;

  if (fp.is_open()) {
    fp << "\"Instruction/Approximation\",\"No approximation\"";
    for (int i = 1; i <= section; i++) {
      fp << ",";
    }
    for (auto& x : icounter_map) {
      uint64_t a = x.first;
      if (a != 0x0) {
        std::string as("");
        for (auto& y: approxes) {
          if ((a & (static_cast<uint64_t>(0x01) << y.first)) != 0x0) {
            as = as + " | " + y.second;
          }
        }
        as.erase(0, 3);
        fp << ", \"" << as << "\"";
        for (int i = 1; i <= section; i++) {
          fp << ", ";
        }
      }
    }
    fp << std::endl;

    for (auto& y : icounter_map) {
      for (int i = 0; i <= section; i++) {
        fp << ",\"Section " << i << "\"";
      }
    }
    fp << std::endl;

    fp << "\"-All instructions-\"";
    uint64_t sum = 0;
    for (int i = 0; i <= section; i++) {
      sum = 0;
      for (uint64_t instr = 0; instr < INSN_COUNT<<2; instr++) {
        sum += icounter_map[0x0][i][instr];
      }
      fp << ", " << sum;
    }
    for (auto& y : icounter_map) {
      if (y.first != 0x0) {
        for (int i = 0; i <= section; i++) {
          sum = 0;
          if (icounter_map[y.first].count(i)) {
            for (uint64_t instr = 0; instr < INSN_COUNT<<2; instr++) {
              sum += icounter_map[y.first][i][instr];
            }
          }
          fp << ", " << sum;
        }
      }
    }
    fp << std::endl;

    for (int prv = 0; prv < 4; prv++) {
      fp << "\"-All|" << prvs[prv] << "-\"";
      for (int i = 0; i <= section; i++) {
        sum = 0;
        for (uint64_t instr = prv; instr < INSN_COUNT<<2; instr+=4) {
          sum += icounter_map[0x0][i][instr];
        }
        fp << ", " << sum;
      }
      for (auto& y : icounter_map) {
        if (y.first != 0x0) {
          for (int i = 0; i <= section; i++) {
            sum = 0;
            if (icounter_map[y.first].count(i)) {
              for (uint64_t instr = prv; instr < INSN_COUNT<<2; instr+=4) {
                sum += icounter_map[y.first][i][instr];
              }
            }
            fp << ", " << sum;
          }
        }
      }
      fp << std::endl;
    }

    for (uint64_t instr = 0; instr < INSN_COUNT<<2; instr++) {
      fp <<"\"" << AxPIKE::Stats::insns[instr>>2] << "|" << prvs[instr&0x3] << "\"";
      for (int i = 0; i <= section; i++) {
        fp << ", " << icounter_map[0x0][i][instr];
      }
      for (auto& y : icounter_map) {
        if (y.first != 0x0) {
          for (int i = 0; i <= section; i++) {
            if (icounter_map[y.first].count(i)) {
              fp << ", " << icounter_map[y.first][i][instr];
            }
            else {
              fp << ", 0";
            }
          }
        }
      }
      fp << std::endl;
    }

    fp.close();
  }
  else {
    std::cerr << "AxPIKE Error: Unable to open file \"" << fname << "\" to save instruction counters." << std::endl;
  }
}

void AxPIKE::Stats::printEnergyCounter(const char* fname) {
  std::ofstream fp;
  
  const char *prvs[4] = {"U", "S", "HS", "M"};

  fp.open(fname, std::ios::out | std::ios::trunc);
  std::cerr << "Writing AxPIKE energy counters to " << fname << std::endl;

  if (fp.is_open()) {
    fp << "\"Instruction/Approximation\",\"No approximation\"";
    for (int i = 1; i <= section; i++) {
      fp << ",";
    }
    for (auto& x : ecounter_map) {
      uint64_t a = x.first;
      if (a != 0x0) {
        std::string as("");
        for (auto& y: approxes) {
          if ((a & (static_cast<uint64_t>(0x01) << y.first)) != 0x0) {
            as = as + " | " + y.second;
          }
        }
        as.erase(0, 3);
        fp << ", \"" << as << "\"";
        for (int i = 1; i <= section; i++) {
          fp << ", ";
        }
      }
    }
    fp << std::endl;

    for (auto& y : ecounter_map) {
      for (int i = 0; i <= section; i++) {
        fp << ",\"Section " << i << "\"";
      }
    }
    fp << std::endl;

    fp << "\"-All instructions-\"";
    double sum = 0;
    for (int i = 0; i <= section; i++) {
      sum = 0;
      for (uint64_t instr = 0; instr < INSN_COUNT<<2; instr++) {
        sum += ecounter_map[0x0][i][instr];
      }
      fp << ", " << sum;
    }
    for (auto& y : ecounter_map) {
      if (y.first != 0x0) {
        for (int i = 0; i <= section; i++) {
          sum = 0;
          if (icounter_map[y.first].count(i)) {
            for (uint64_t instr = 0; instr < INSN_COUNT<<2; instr++) {
              sum += ecounter_map[y.first][i][instr];
            }
          }
          fp << ", " << sum;
        }
      }
    }
    fp << std::endl;

    for (int prv = 0; prv < 4; prv++) {
      fp << "\"-All|" << prvs[prv] << "-\"";
      for (int i = 0; i <= section; i++) {
        sum = 0;
        for (uint64_t instr = prv; instr < INSN_COUNT<<2; instr+=4) {
          sum += ecounter_map[0x0][i][instr];
        }
        fp << ", " << sum;
      }
      for (auto& y : ecounter_map) {
        if (y.first != 0x0) {
          for (int i = 0; i <= section; i++) {
            sum = 0;
            if (icounter_map[y.first].count(i)) {
              for (uint64_t instr = prv; instr < INSN_COUNT<<2; instr+=4) {
                sum += ecounter_map[y.first][i][instr];
              }
            }
            fp << ", " << sum;
          }
        }
      }
      fp << std::endl;
    }

    for (uint64_t instr = 0; instr < INSN_COUNT<<2; instr++) {
      fp <<"\"" << AxPIKE::Stats::insns[instr>>2] << "|" << prvs[instr&0x3] << "\"";
      for (int i = 0; i <= section; i++) {
        fp << ", " << ecounter_map[0x0][i][instr];
      }
      for (auto& y : ecounter_map) {
        if (y.first != 0x0) {
          for (int i = 0; i <= section; i++) {
            if (icounter_map[y.first].count(i)) {
              fp << ", " << ecounter_map[y.first][i][instr];
            }
            else {
              fp << ", 0.0";
            }
          }
        }
      }
      fp << std::endl;
    }

    fp.close();
  }
  else {
    std::cerr << "AxPIKE Error: Unable to open file \"" << fname << "\" to save energy counters." << std::endl;
  }
}

void AxPIKE::Stats::insnDispatch(double energy) {
  instrs_counter++;

  uint64_t instr = (cur_insn_id<<2) | p->state.prv;
  (icounter[instr])++;
  (ecounter[instr]) += energy;
}

void AxPIKE::Stats::setStats() {
  uint64_t& prv = p->state.prv;
  icounter = icounter_map[active_approx[prv]][section];
  ecounter = ecounter_map[active_approx[prv]][section];
  if (!icounter) {
    icounter = new uint64_t[INSN_COUNT<<2];
    ecounter = new double[INSN_COUNT<<2];
    for (int i = 0; i < (INSN_COUNT<<2); i++) {
      icounter[i] = 0;
      ecounter[i] = 0.0;
    }
    icounter_map[active_approx[prv]][section] = icounter;
    ecounter_map[active_approx[prv]][section] = ecounter;
  }
}
