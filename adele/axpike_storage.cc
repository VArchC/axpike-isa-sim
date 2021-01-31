#include "common.h"
#include "axpike_storage.h"
#include "axpike_control.h"

template class AxPIKE::Word<reg_t>;
template class AxPIKE::Word<sreg_t>;
template class AxPIKE::Word<freg_t>;

template <typename T> void AxPIKE::Word<T>::set(AxPIKE::Control* _c, AxPIKE::Source::type_t _type, std::string _name, uint32_t _address) {
  c = _c;
  type = _type;
  name = _name;
  address = _address;
  if (_type == Source::REGBANK) {
    DM_rd = (c->DM_rbnrd);
    DM_wr = (c->DM_rbnwr);
  }
  if (_type == Source::REG) {
    DM_rd = (c->DM_regrd);
    DM_wr = (c->DM_regwr);
  }
}

template <typename T> void AxPIKE::Word<T>::write (T _value) {
  if (likely(DM_wr != NULL) && DM_wr[c->cur_insn_id] != NULL) {
    source_t s(type, AxPIKE::Source::NA, name, address, address, source_t::WRITE, sizeof(T), &(this->value));
    (DM_wr[c->cur_insn_id])(c->p, c->insn, c->pc, c->xlen, c->npc, &s, &_value);
  }
  this->value = _value;
}

template <typename T> T AxPIKE::Word<T>::read () const {
  T faulty = this->value;
  if (likely(DM_rd != NULL) && DM_rd[c->cur_insn_id] != NULL) {
    source_t s(type, AxPIKE::Source::NA, name, address, address, source_t::READ, sizeof(T), &faulty);
    (DM_rd[c->cur_insn_id])(c->p, c->insn, c->pc, c->xlen, c->npc, &s, &faulty);
  }
  return faulty;
}

template <typename T> AxPIKE::Word<T>& AxPIKE::Word<T>::operator = (const Word& _value) {
  T __value = _value;
  this->write(__value);
  return *this;
}

template <typename T> AxPIKE::Word<T>& AxPIKE::Word<T>::operator = (const T& _value) {
  this->write(_value);
  return *this;
}

template <typename T> AxPIKE::Word<T>::operator T() const {
  return this->read();
}

