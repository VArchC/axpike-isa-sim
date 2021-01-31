#ifndef _AXPIKE_HTIF_H_
#define _AXPIKE_HTIF_H_

#include <unordered_map>
#include "fesvr/device.h"

class htif_t;
class sim_t;
class processor_t;

namespace AxPIKE {
  class HTIF : public device_t {
    public:
      HTIF();
      void initialize(sim_t* _sim);

    private:
      void handle_control(command_t cmd);
      const char* identity() {return "AxPIKE_HTIF"; }
      sim_t* sim;
      std::unordered_map<uint32_t, processor_t*> harts;
  };
}

#endif
