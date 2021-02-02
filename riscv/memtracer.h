// See LICENSE for license details.

#ifndef _MEMTRACER_H
#define _MEMTRACER_H

#include <cstdint>
#include <string.h>
#include <vector>

enum access_type {
  LOAD,
  STORE,
  FETCH,
};

#define CACHE_LEVEL_L2 2
#define CACHE_LEVEL_IC 1
#define CACHE_LEVEL_DC 0

struct memtracer_log_t {
  bool dc_miss;
  bool ic_miss;
  bool l2_miss;
  uint64_t wb_address;
};

class memtracer_t
{
 public:
  memtracer_t() {}
  virtual ~memtracer_t() {}

  virtual bool interested_in_range(uint64_t begin, uint64_t end, access_type type) = 0;
  virtual void trace(uint64_t addr, size_t bytes, access_type type, memtracer_log_t *log) = 0;
};

class memtracer_list_t : public memtracer_t
{
 public:
  bool empty() { return list.empty(); }
  bool interested_in_range(uint64_t begin, uint64_t end, access_type type)
  {
    for (std::vector<memtracer_t*>::iterator it = list.begin(); it != list.end(); ++it)
      if ((*it)->interested_in_range(begin, end, type))
        return true;
    return false;
  }
  void trace(uint64_t addr, size_t bytes, access_type type, memtracer_log_t *log)
  {
    for (std::vector<memtracer_t*>::iterator it = list.begin(); it != list.end(); ++it)
      (*it)->trace(addr, bytes, type, log);
  }
  void hook(memtracer_t* h)
  {
    list.push_back(h);
  }
 private:
  std::vector<memtracer_t*> list;
};

#endif
