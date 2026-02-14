/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 TheSuperHackers
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// This file contains string macros and alias functions to help compiling on
// non-windows platforms
#pragma once
#include <ctype.h>
#include <string.h>
#if !defined(_WIN32)
#include <strings.h>
#endif

#ifndef _LPCSTR_DEFINED
#define _LPCSTR_DEFINED
typedef const char *LPCSTR;
typedef char *LPSTR;
#endif

#ifdef __cplusplus
extern "C" {
#endif

// String functions
inline char *_strlwr(char *str) {
  if (!str)
    return NULL;
  for (int i = 0; str[i] != '\0'; i++) {
    str[i] = (char)tolower((unsigned char)str[i]);
  }
  return str;
}

inline char *_strupr(char *str) {
  if (!str)
    return NULL;
  for (int i = 0; str[i] != '\0'; i++) {
    str[i] = (char)toupper((unsigned char)str[i]);
  }
  return str;
}

#ifdef __cplusplus
}
#endif

#ifndef strlwr
#define strlwr _strlwr
#endif
#ifndef strupr
#define strupr _strupr
#endif

// Do NOT define _strlwr and _strupr as macros as they might be declared as
// functions in other headers (like GameSpy) #define _strlwr _strlwr #define
// _strupr _strupr

#define stricmp strcasecmp
#define strnicmp strncasecmp
#define _stricmp strcasecmp
#define _strnicmp strncasecmp
#define strcmpi strcasecmp

#ifdef __cplusplus
extern "C" {
#endif

inline char *_strdup(const char *lpString) { return strdup(lpString); }

inline char *lstrcpyn(char *lpString1, const char *lpString2, int iMaxLength) {
  if (iMaxLength <= 0)
    return lpString1;
  strncpy(lpString1, lpString2, iMaxLength);
  lpString1[iMaxLength - 1] = '\0';
  return lpString1;
}

inline char *lstrcat(char *lpString1, const char *lpString2) {
  return strcat(lpString1, lpString2);
}

inline char *lstrcpy(char *lpString1, const char *lpString2) {
  return strcpy(lpString1, lpString2);
}

inline int lstrcmpi(LPCSTR lpString1, LPCSTR lpString2) {
  return strcasecmp(lpString1, lpString2);
}

#ifdef __cplusplus
}
#endif

#define lstrlen strlen
#define lstrcmp strcmp
