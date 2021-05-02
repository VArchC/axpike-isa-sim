#ifndef _AXPIKE_CONTROL_H_
#define _AXPIKE_CONTROL_H_

#include "decode.h"
#include <list>
#include <unordered_map>
#include <vector>
#include <string>
#include <limits>
#include "axpike_stats.h"
#include "axpike_storage.h"
#include "axpike_log.h"
#include "insn_count.h"

class processor_t;

namespace AxPIKE {
  typedef void (*wrapper_t)(processor_t*, insn_t, reg_t, int, reg_t, source_t*, void*);

  class WrapperElement {
    public:
      uint8_t approx_id;
      wrapper_t func;
      bool operator== (const WrapperElement& cmp) {return (cmp.approx_id == approx_id);}
      WrapperElement(uint8_t _approx_id, wrapper_t _func) : approx_id(_approx_id), func(_func) {}
      WrapperElement(uint8_t _approx_id) : approx_id(_approx_id) {}
  };

  class WrapperList {
    private:
      std::list<WrapperElement> wrappers;
      wrapper_t func;
    public:
      WrapperList() : wrappers(), func(NULL) {}
      void operator() (processor_t* p, insn_t insn, reg_t pc, int xlen, reg_t npc, source_t* source, void* data) {(*func)(p, insn, pc, xlen, npc, source, data);}
      bool operator!= (const void* rhs) {return func != rhs;}
      bool operator== (const void* rhs) {return func == rhs;}
      void activateModel(uint8_t approx_id, wrapper_t func);
      void deactivateModel(uint8_t approx_id);
      void clear();
  };

  typedef std::unordered_map<std::string, double> op_t;

  class OPElement {
    public:
      uint8_t approx_id;
      op_t params;
      bool operator== (const OPElement& cmp) {return (cmp.approx_id == approx_id);}
      OPElement(uint8_t _approx_id, const OPElement& previous, op_t _params);
      OPElement(uint8_t _approx_id, op_t _params) : approx_id(_approx_id), params(_params) {};
      OPElement(uint8_t _approx_id) : approx_id(_approx_id) {};
      double operator[] (const std::string& key);
  };

  class OPList {
    private:
      std::unordered_map<unsigned int, std::list<OPElement>> params;
      uint32_t& cur_insn_id;
      uint64_t& prv;
    public:
      OPList(uint32_t& _cur_insn_id, uint64_t& _prv) : params(), cur_insn_id(_cur_insn_id), prv(_prv) {};
      double operator[] (const std::string& key);
      void activateOP(uint32_t insn, uint64_t prv, uint8_t approx_id, op_t params);
      void deactivateOP(uint8_t approx_id, uint64_t prv);
      void clear();
  };

  class Control {
    protected:
      std::string app;
    private:
      // Setup
      void initModels();
    public:
      uint64_t active_approx[4];
      uint32_t cur_insn_id;
      uint64_t& prv;
      Stats stats;
      Source s;
      Log log;
      CustomCounters::Counters counters;

      processor_t* p;
      insn_t insn;
      reg_t pc;
      int xlen;
      reg_t npc;
      
      WrapperList IM[INSN_COUNT << 2];
      WrapperList EM[INSN_COUNT << 2];
      WrapperList DM_regrd[INSN_COUNT << 2];
      WrapperList DM_regwr[INSN_COUNT << 2];
      WrapperList DM_rbnrd[INSN_COUNT << 2];
      WrapperList DM_rbnwr[INSN_COUNT << 2];
      WrapperList DM_memrd[INSN_COUNT << 2];
      WrapperList DM_memwr[INSN_COUNT << 2];
     
      OPList OP;

      Control(processor_t* _p, std::vector<std::string> adele_activate, std::vector<std::string> adele_deactivate);

      void issue(uint8_t cmd);

      void activateApprox(uint8_t approx_id, uint64_t prv);
      void deactivateApprox(uint8_t approx_id, uint64_t prv);
      
      std::unordered_map<std::string, uint8_t> approxes;
  };
}

#endif
