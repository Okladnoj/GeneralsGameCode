/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 TheSuperHackers
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
*/

#pragma once

#ifndef _WIN32
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef _O_RDONLY
#define _O_RDONLY O_RDONLY
#endif
#ifndef _O_WRONLY
#define _O_WRONLY O_WRONLY
#endif
#ifndef _O_RDWR
#define _O_RDWR O_RDWR
#endif
#ifndef _O_APPEND
#define _O_APPEND O_APPEND
#endif
#ifndef _O_CREAT
#define _O_CREAT O_CREAT
#endif
#ifndef _O_TRUNC
#define _O_TRUNC O_TRUNC
#endif
#ifndef _O_TEXT
#define _O_TEXT 0
#endif
#ifndef _O_BINARY
#define _O_BINARY 0
#endif

#ifndef _S_IREAD
#define _S_IREAD S_IRUSR
#endif
#ifndef _S_IWRITE
#define _S_IWRITE S_IWUSR
#endif

#define _open open
#define _close close
#define _read read
#define _write write
#define _lseek lseek
#define _access access
#define _commit fsync
#define _fileno fileno

#endif
