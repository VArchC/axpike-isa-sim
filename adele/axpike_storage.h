#ifndef _AXPIKE_STORAGE_H_
#define _AXPIKE_STORAGE_H_

#include <string>
#include <unordered_map>

#include "memtracer.h"

namespace AxPIKE {
  class Control;
  class WrapperList;
    
  struct Source {
    static const uint8_t NA = 0;
    static const uint8_t L1 = 1;
    static const uint8_t L2 = 2;
    static const uint8_t L3 = 4;
    static const uint8_t TLB = 8;
    typedef enum {MEM, REGBANK, REG} type_t;
    typedef enum {READ, WRITE} op_t;
    Source() {};
    Source(type_t _type, memtracer_log_t _memtracer_log, std::string _name, uint64_t _address, uint64_t _paddress, op_t _op, size_t _width, void* _bypass) : type(_type), memtracer_log(_memtracer_log), name(_name), address(_address), paddress(_paddress), op(_op), width(_width), bypass(_bypass) {};
    type_t type;
    memtracer_log_t memtracer_log;
    std::string name;
    uint64_t address;
    uint64_t paddress;
    uint64_t wb_address;
    op_t op;
    size_t width;
    void* bypass;
  };
  
  template <typename T>
  class Word {
    private:
      Control* c;
      Source::type_t type;
      std::string name;
      uint32_t address;
      WrapperList* DM_rd;
      WrapperList* DM_wr;
     
      void write(T _value);
      T read() const;

      //Word(const Word&) = delete;
    public:
      T value;

      Word() : c(NULL), DM_rd(NULL), DM_wr(NULL) {};
      Word(const Word& cp) : c(NULL), DM_rd(NULL), DM_wr(NULL) {this->value = cp.read();};
      Word(Control* _c, Source::type_t _type, std::string _name, uint32_t _address, WrapperList* _DM_rd, WrapperList* _DM_wr) : c(_c), type(_type), name(_name), address(_address), DM_rd(_DM_rd), DM_wr(_DM_wr) {};

      void set(Control* _c, Source::type_t _type, std::string _name, uint32_t _address);

      Word& operator = (const Word& _value);
      Word& operator = (const T& _value);

      operator T() const;
  };
}

typedef AxPIKE::Source source_t;

#endif
