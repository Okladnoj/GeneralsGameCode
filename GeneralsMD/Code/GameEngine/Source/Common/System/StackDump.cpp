/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
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

//////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.
//																																						//
//////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h" // This must go first in EVERY cpp file in the GameEngine
#include <string.h>

#ifdef _WIN32
#if defined(RTS_DEBUG) || defined(IG_DEBUG_STACKTRACE)
// NOTE: Original Windows-specific code would be here.
// For now, we are stubbing this out on macOS.
#endif
#else
#include "Common/AsciiString.h"
#include "Common/Debug.h"
#include "Common/StackDump.h"
#include <Utility/win_compat.h>

AsciiString g_LastErrorDump;

void StackDump(void (*callback)(const char *)) {}
void StackDumpFromContext(DWORD eip, DWORD esp, DWORD ebp,
                          void (*callback)(const char *)) {}
BOOL InitSymbolInfo() { return FALSE; }
void MakeStackTrace(DWORD myeip, DWORD myesp, DWORD myebp, int skipFrames,
                    void (*callback)(const char *)) {}
void GetFunctionDetails(void *pointer, char *name, char *filename,
                        unsigned int *linenumber, unsigned int *address) {
  if (name)
    strcpy(name, "<Unknown>");
  if (filename)
    strcpy(filename, "<Unknown>");
  if (linenumber)
    *linenumber = 0xFFFFFFFF;
  if (address)
    *address = 0xFFFFFFFF;
}
void FillStackAddresses(void **addresses, unsigned int count,
                        unsigned int skip) {
  for (unsigned int i = 0; i < count; ++i)
    addresses[i] = nullptr;
}
void StackDumpFromAddresses(void **addresses, unsigned int count,
                            void (*callback)(const char *)) {}
void WriteStackLine(void *address, void (*callback)(const char *)) {}
void DumpExceptionInfo(unsigned int u, EXCEPTION_POINTERS *e_info) {}

#endif
