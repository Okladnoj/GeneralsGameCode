#ifndef COMIP_H
#define COMIP_H

#include "win_compat.h"

// Stub for __uuidof
#define __uuidof(x) GUID_NULL

template <typename T, const GUID *iid> struct my_com_IIID {
  typedef T Interface;
};

template <typename IIID> class my_com_ptr_t {
public:
  typedef typename IIID::Interface Interface;

  my_com_ptr_t() : p(nullptr) {}
  my_com_ptr_t(int) : p(nullptr) {}
  my_com_ptr_t(Interface *_p) : p(_p) {}

  Interface *operator->() const { return p; }
  Interface &operator*() const { return *p; }
  operator Interface *() const { return p; }

  bool operator==(int null) const { return p == nullptr; }
  bool operator!=(int null) const { return p != nullptr; }

  my_com_ptr_t &operator=(int null) {
    p = nullptr;
    return *this;
  }

  HRESULT CreateInstance(const GUID &guid) { return E_FAIL; }

  void Release() { p = nullptr; }

private:
  Interface *p;
};

#endif // COMIP_H
