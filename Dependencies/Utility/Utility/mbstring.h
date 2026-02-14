#ifndef MBSTRING_COMPAT_H
#define MBSTRING_COMPAT_H

#include <stddef.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// Stub for _mbsnccnt
// Returns the number of characters in the first 'n' bytes of multibyte string
// 's'. For now, assume simple mapping or UTF-8. A very crude version that just
// returns 'n' (assuming ASCII/already converted) or we can implement a basic
// UTF-8 char count if that's what we expect.
inline size_t _mbsnccnt(const unsigned char *s, size_t n) {
  if (!s)
    return 0;
  size_t count = 0;
  size_t bytes = 0;
  while (bytes < n && s[bytes]) {
    unsigned char c = s[bytes];
    if (c < 0x80) {
      bytes += 1;
    } else if ((c & 0xE0) == 0xC0) {
      bytes += 2;
    } else if ((c & 0xF0) == 0xE0) {
      bytes += 3;
    } else if ((c & 0xF8) == 0xF0) {
      bytes += 4;
    } else {
      // Invalid UTF-8 or other MBCS, just skip 1 byte
      bytes += 1;
    }
    if (bytes <= n) {
      count++;
    }
  }
  return count;
}

#ifdef __cplusplus
}
#endif

#endif
