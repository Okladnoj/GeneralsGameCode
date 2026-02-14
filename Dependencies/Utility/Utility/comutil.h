#ifndef COMUTIL_H
#define COMUTIL_H

#include "win_compat.h"

// Basic _bstr_t stub
class _bstr_t {
public:
  _bstr_t() {}
  _bstr_t(const char *s) {}
  _bstr_t(const WCHAR *s) {}
  operator const char *() const { return ""; }
  operator const WCHAR *() const { return u""; }
};

// Basic _variant_t stub if needed
class _variant_t {
public:
  _variant_t() {}
};

#endif // COMUTIL_H
