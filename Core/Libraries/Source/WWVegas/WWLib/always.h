/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
*/

#pragma once

#include <Utility/CppMacros.h>

#ifdef __APPLE__
#define RA_MACOS
#include <MacOSSystemProtection.h>
#endif

#ifndef _WIN32
#include <Utility/win_compat.h>
#else
#include <tchar.h>
#include <windows.h>
#endif

#ifndef interface
#ifndef __OBJC__
#define interface struct
#endif
#endif

#ifndef BOOL
#ifdef __APPLE__
#define BOOL BOOL
#else
typedef int BOOL;
#define BOOL BOOL
#endif
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#include "Lib/BaseType.h"
#include <Utility/dbghelp.h>
#include <Utility/thread_compat.h>

#ifdef _WIN32
#include <crtdbg.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#ifndef __stdcall
#define __stdcall
#endif
#ifndef __cdecl
#define __cdecl
#endif
#ifndef __forceinline
#define __forceinline inline __attribute__((always_inline))
#endif
#define WWINLINE __forceinline
#define _stricmp strcasecmp
#define _strnicmp strncasecmp
#define _vsnprintf vsnprintf
#define _fullpath(a, b, c) realpath(b, a)
#define _splitpath(path, drive, dir, fname, ext) // dummy
#endif
#define SET_REF_OWNER(P) P

#include "Common/GameMemory.h"

#ifndef MEMORY_POOL_GLUE
#define MEMORY_POOL_GLUE(Class, PoolName)                                      \
public:                                                                        \
  enum { Class##_GLUE_NOT_IMPLEMENTED = 0 };
#endif

#ifndef W3DMPO
#define W3DMPO MemoryPoolObject
#endif

#ifndef W3DMPO_GLUE
#ifdef RA_MACOS
#define W3DMPO_GLUE(x)                                                         \
  MEMORY_POOL_GLUE_WITHOUT_GCMP_NO_DTOR(x)                                     \
  GCMP_CREATE(x, #x, -1, -1)
#else
#define W3DMPO_GLUE(x)                                                         \
  MEMORY_POOL_GLUE_WITHOUT_GCMP_NO_DTOR(x)                                     \
  GCMP_FIND(x, #x)
#endif
#endif

#ifdef RA_MACOS
#ifndef NEW_REF
#define NEW_REF(target, constructor) new (__FILE__, __LINE__) target constructor
#endif

#ifndef W3DNEW
#define W3DNEW new (__FILE__, __LINE__)
#endif

#ifndef W3DNEWARRAY
#define W3DNEWARRAY new
#endif
#else
#ifndef NEW_REF
#define NEW_REF(target, constructor) new target constructor
#endif

#ifndef W3DNEW
#define W3DNEW new
#endif

#ifndef W3DNEWARRAY
#define W3DNEWARRAY new
#endif
#endif

#ifndef MSGW3DNEW
#define MSGW3DNEW(msg) new
#endif

#ifndef MSGW3DNEWARRAY
#define MSGW3DNEWARRAY(msg) new
#endif

#include "../WWSaveLoad/persist.h"
#include "WWCommon.h"
#include "refcount.h"
