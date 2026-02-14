#pragma once
#include "stdint_adapter.h"
#include <stddef.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

// WCHAR
#if defined(_WIN32)
typedef wchar_t WCHAR;
#elif defined(__cplusplus)
typedef char16_t WCHAR;
#else
typedef uint16_t WCHAR;
#endif
typedef const WCHAR *LPCWSTR;
typedef WCHAR *LPWSTR;

#ifdef __cplusplus
extern "C" {
#endif

inline size_t wcslen_u16(const WCHAR *s) {
  size_t len = 0;
  while (s[len])
    len++;
  return len;
}

inline int wcscmp_u16(const WCHAR *s1, const WCHAR *s2) {
  while (*s1 && (*s1 == *s2)) {
    s1++;
    s2++;
  }
  return (int)*s1 - (int)*s2;
}

inline WCHAR *wcscpy_u16(WCHAR *dest, const WCHAR *src) {
  WCHAR *d = dest;
  while ((*d++ = *src++))
    ;
  return dest;
}

inline int wcsicmp_u16(const WCHAR *s1, const WCHAR *s2) {
  while (*s1 && (tolower((int)*s1) == tolower((int)*s2))) {
    s1++;
    s2++;
  }
  return tolower((int)*s1) - tolower((int)*s2);
}

inline WCHAR *wcschr_u16(const WCHAR *s, WCHAR c) {
  while (*s && *s != c)
    s++;
  return (*s == c) ? (WCHAR *)s : NULL;
}

inline WCHAR *wcsrchr_u16(const WCHAR *s, WCHAR c) {
  const WCHAR *last = NULL;
  while (*s) {
    if (*s == c)
      last = s;
    s++;
  }
  return (WCHAR *)last;
}

inline WCHAR *wcscat_u16(WCHAR *dest, const WCHAR *src) {
  WCHAR *d = dest;
  while (*d)
    d++;
  while ((*d++ = *src++))
    ;
  return dest;
}

inline WCHAR *wcsstr_u16(const WCHAR *haystack, const WCHAR *needle) {
  if (!*needle)
    return (WCHAR *)haystack;
  while (*haystack) {
    const WCHAR *h = haystack;
    const WCHAR *n = needle;
    while (*h && *n && *h == *n) {
      h++;
      n++;
    }
    if (!*n)
      return (WCHAR *)haystack;
    haystack++;
  }
  return NULL;
}

inline WCHAR *wcsncpy_u16(WCHAR *dest, const WCHAR *src, size_t n) {
  WCHAR *d = dest;
  while (n > 0 && *src) {
    *d++ = *src++;
    n--;
  }
  while (n > 0) {
    *d++ = 0;
    n--;
  }
  return dest;
}

inline size_t wcsspn_u16(const WCHAR *s, const WCHAR *accept) {
  const WCHAR *p = s;
  const WCHAR *a;
  size_t count = 0;
  while (*p) {
    for (a = accept; *a; a++) {
      if (*p == *a)
        break;
    }
    if (!*a)
      return count;
    p++;
    count++;
  }
  return count;
}

inline size_t wcscspn_u16(const WCHAR *s, const WCHAR *reject) {
  const WCHAR *p = s;
  const WCHAR *r;
  size_t count = 0;
  while (*p) {
    for (r = reject; *r; r++) {
      if (*p == *r)
        return count;
    }
    p++;
    count++;
  }
  return count;
}
#ifdef __cplusplus
}
#endif

#include <stdarg.h>
#include <stdio.h>

inline int vswprintf_u16(WCHAR *str, size_t size, const WCHAR *format,
                         va_list ap) {
  // Basic implementation: convert format to ASCII and use vsnprintf
  char fmt[1024];
  size_t i = 0;
  while (i < 1023 && format[i]) {
    fmt[i] = (char)format[i];
    i++;
  }
  fmt[i] = '\0';

  char buf[4096];
  int res = vsnprintf(buf, sizeof(buf), fmt, ap);
  if (res >= 0) {
    size_t j = 0;
    size_t limit = (size < (size_t)res + 1) ? size - 1 : (size_t)res;
    while (j < limit) {
      str[j] = (WCHAR)buf[j];
      j++;
    }
    str[j] = 0;
  }
  return res;
}

inline int swprintf_u16(WCHAR *str, size_t size, const WCHAR *format, ...) {
  va_list ap;
  va_start(ap, format);
  int res = vswprintf_u16(str, size, format, ap);
  va_end(ap);
  return res;
}

#ifndef _WIN32
#define wcslen wcslen_u16
#define wcscmp wcscmp_u16
#define wcscpy wcscpy_u16
#define wcscat wcscat_u16
#define wcschr wcschr_u16
#define wcsrchr wcsrchr_u16
#define wcsstr wcsstr_u16
#define wcsncpy wcsncpy_u16
#define wcsspn wcsspn_u16
#define wcscspn wcscspn_u16
#define _wcsicmp wcsicmp_u16
#define wcsicmp wcsicmp_u16
#define vswprintf vswprintf_u16
#define swprintf swprintf_u16
#endif

// MultiByteToWideChar
#define CP_ACP 0
#define CP_UTF8 65001

#ifdef __cplusplus
extern "C" {
#endif

inline int MultiByteToWideChar(unsigned int cp, unsigned int flags,
                               const char *mbstr, int cb, WCHAR *wcstr,
                               int cch) {
  if (cb == -1)
    cb = (int)strlen(mbstr) + 1;
  if (cch == 0)
    return cb;
  int i;
  for (i = 0; i < cb && i < cch; ++i) {
    wcstr[i] = (WCHAR)(unsigned char)mbstr[i];
  }
  return i;
}

inline int WideCharToMultiByte(unsigned int cp, unsigned int flags,
                               const WCHAR *wcstr, int cch, char *mbstr, int cb,
                               const char *defchar, int *used) {
  if (cch == -1) {
    cch = 0;
    while (wcstr[cch])
      cch++;
    cch++; // including null
  }
  if (cb == 0)
    return cch;
  int i;
  for (i = 0; i < cch && i < cb; ++i) {
    mbstr[i] = (char)wcstr[i];
  }
  return i;
}

#ifdef __cplusplus
}
#endif
